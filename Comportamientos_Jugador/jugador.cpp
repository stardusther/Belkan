#include "../Comportamientos_Jugador/jugador.hpp"
#include "motorlib/util.h"

#include <iostream>
#include <cmath>
#include <set>
#include <stack>
#include <vector>
#include <queue>


// Este es el método principal que se piden en la practica.
// Tiene como entrada la información de los sensores y devuelve la acción a realizar.
// Para ver los distintos sensores mirar fichero "comportamiento.hpp"
Action ComportamientoJugador::think(Sensores sensores) {

    actual.fila        = sensores.posF;
    actual.columna     = sensores.posC;
    actual.orientacion = sensores.sentido;

    objetivos.clear(); // Vaciamos la lista de objetivos
    for(int i=0; i<sensores.num_destinos; i++){
        estado aux;
        aux.fila = sensores.destino[2*i];
        aux.columna = sensores.destino[2*i+1];
        objetivos.push_back(aux);
    }

    if (!hayPlan){
        hayPlan = pathFinding(sensores.nivel, actual, objetivos, plan);
    }

    Action sigAccion = actIDLE; // lo inicialiazmos a actIDLE por si acaso
    if(hayPlan and plan.size()>0){
        sigAccion = plan.front();
        plan.erase(plan.begin());
    }
    else{
        cout << "No se pudo encontrar un plan\n";
    }

    return sigAccion;



}

// Llama al algoritmo de busqueda que se usara en cada comportamiento del agente
// Level representa el comportamiento en el que fue iniciado el agente.
bool ComportamientoJugador::pathFinding (int level, const estado &origen, const list<estado> &destino, list<Action> &plan){
    estado un_objetivo;
    un_objetivo = objetivos.front();
    cout << "fila: " << un_objetivo.fila << " col:" << un_objetivo.columna << endl;

	switch (level){
		case 0: cout << "Demo\n";
                        return pathFinding_Profundidad(origen,un_objetivo,plan);
						break;

		case 1: cout << "Optimo numero de acciones\n";
                        return pathFinding_Anchura(origen,un_objetivo,plan);
						break;
		case 2: cout << "Optimo en coste 1 Objetivo\n";
						return pathFinding_CosteUniforme(origen,un_objetivo,plan);
						break;
		case 3: cout << "Optimo en coste 3 Objetivos\n";
                        cout << "No implementado aun\n";
						// Incluir aqui la llamada al algoritmo de busqueda para 3 objetivos
						break;
		case 4: cout << "Algoritmo de busqueda usado en el reto\n";
						// Incluir aqui la llamada al algoritmo de busqueda usado en el nivel 2
						cout << "No implementado aun\n";
						break;
	}
	return false;
}


//---------------------- Implementación de la busqueda de costo uniforme ---------------------------

// Devuelve el coste de avanzar en una casilla en el mapa
int ComportamientoJugador::getCoste(estado &hijo){
	int coste = 1;
	unsigned char casilla = mapaResultado[hijo.fila][hijo.columna];

    if(casilla == 'A' && hijo.bikini == true)
        coste = 10;
    else if (casilla == 'A' && hijo.bikini == false)
        coste = 200;

    if(casilla == 'B' && hijo.zapatillas == true)
        coste = 15;
    else if (casilla == 'B' && hijo.zapatillas == false)
        coste = 100;

    if(casilla == 'T')
        coste = 2;

	return coste;
}

// Devuelve el coste de girar en una casilla en el mapa
int ComportamientoJugador::getCosteGiro(estado &hijo){
	int coste = 1;
	unsigned char casilla = mapaResultado[hijo.fila][hijo.columna];

    if(casilla == 'A' && hijo.bikini == true)
        coste = 5;
    else if(casilla == 'A' && hijo.bikini == false)
        coste = 500;

    if(casilla == 'B' && hijo.zapatillas == true)
        coste = 1;
    else if (casilla == 'B' && hijo.zapatillas == false)
        coste = 3;

    if(casilla == 'T')
        coste = 2;

	return coste;
}


// Dado el codigo en caracter de una casilla del mapa dice si se puede
// pasar por ella sin riegos de morir o chocar.
bool EsObstaculo(unsigned char casilla){
	if (casilla=='P' or casilla=='M')
		return true;
	else
	  return false;
}

// Comprueba si la casilla que hay delante es un obstaculo. Si es un
// obstaculo devuelve true. Si no es un obstaculo, devuelve false y
// modifica st con la posición de la casilla del avance.
bool ComportamientoJugador::HayObstaculoDelante(estado &st){
	int fil=st.fila, col=st.columna;

  // calculo cual es la casilla de delante del agente
	switch (st.orientacion) {
		case 0: fil--; break;
		case 1: col++; break;
		case 2: fil++; break;
		case 3: col--; break;
	}

	// Compruebo que no me salgo fuera del rango del mapa
	if (fil<0 or fil>=mapaResultado.size()) return true;
	if (col<0 or col>=mapaResultado[0].size()) return true;

	// Miro si en esa casilla hay un obstaculo infranqueable
	if (!EsObstaculo(mapaResultado[fil][col])){
		// No hay obstaculo, actualizo el parametro st poniendo la casilla de delante.
        st.fila = fil;
		st.columna = col;
		return false;
	}
	else{
	  return true;
	}
}

struct nodo{
	estado st;
	int coste = 1;
	list<Action> secuencia;
};

struct ComparaEstados{
  bool operator()(const estado &a, const estado &n) const{
    if ((a.fila > n.fila)
        or
            (a.fila == n.fila and a.columna > n.columna)
        or
            (a.fila == n.fila and a.columna == n.columna and a.orientacion > n.orientacion)
        or
            (a.fila == n.fila and a.columna == n.columna and a.orientacion == n.orientacion
        and
            a.bikini > n.bikini)
        or
            (a.fila == n.fila and a.columna == n.columna
        and
            a.orientacion == n.orientacion
        and
            a.bikini ==  n.bikini
        and
            a.zapatillas > n.zapatillas))
      return true;
    else
      return false;
  }
};


struct ComparaCoste{ // Función de comparación para la cola con prioridad (costo Uniforme)
    bool operator() (const nodo &a, const nodo &b) const{
        if (a.coste > b.coste )
            return true;
        else
            return false;
    }
};

// ----------------------------- IMPLEMENTACIÓN DE LOS DISTINTOS PATHFINDING ----------------------------------------

// Implementación de la busqueda en profundidad.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Profundidad(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> Cerrados; // Lista de Cerrados
	stack<nodo> Abiertos;				 // Lista de Abiertos

  	nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

  while (!Abiertos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		Abiertos.pop();
		Cerrados.insert(current.st);

		// Generar descendiente de girar a la derecha
		nodo hijoTurnR = current;
		hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
		if (Cerrados.find(hijoTurnR.st) == Cerrados.end()){
			hijoTurnR.secuencia.push_back(actTURN_R);
			Abiertos.push(hijoTurnR);

		}

		// Generar descendiente de girar a la izquierda
		nodo hijoTurnL = current;
		hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
		if (Cerrados.find(hijoTurnL.st) == Cerrados.end()){
			hijoTurnL.secuencia.push_back(actTURN_L);
			Abiertos.push(hijoTurnL);
		}

		// Generar descendiente de avanzar
		nodo hijoForward = current;
		if (!HayObstaculoDelante(hijoForward.st)){
			if (Cerrados.find(hijoForward.st) == Cerrados.end()){
				hijoForward.secuencia.push_back(actFORWARD);
				Abiertos.push(hijoForward);
			}
		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty()){
			current = Abiertos.top();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}


// Implementación de la busqueda en anchura.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_Anchura(const estado &origen, const estado &destino, list<Action> &plan) {
	//Borro la lista
	cout << "Calculando plan\n";
	plan.clear();
	set<estado,ComparaEstados> Cerrados; // Lista de Cerrados
	queue<nodo> Abiertos;				 // Cola de Abiertos

    nodo current;
	current.st = origen;
	current.secuencia.empty();

	Abiertos.push(current);

  while (!Abiertos.empty() and (current.st.fila!=destino.fila or current.st.columna != destino.columna)){

		Abiertos.pop();

		if(Cerrados.find(current.st) == Cerrados.end()){	// optimización
			Cerrados.insert(current.st);

            // Generar descendiente de girar a la derecha
            nodo hijoTurnR = current;
            hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;
            if (Cerrados.find(hijoTurnR.st) == Cerrados.end()){
                hijoTurnR.secuencia.push_back(actTURN_R);
                Abiertos.push(hijoTurnR);

            }

            // Generar descendiente de girar a la izquierda
            nodo hijoTurnL = current;
            hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;
            if (Cerrados.find(hijoTurnL.st) == Cerrados.end()){
                hijoTurnL.secuencia.push_back(actTURN_L);
                Abiertos.push(hijoTurnL);
            }

            // Generar descendiente de avanzar
            nodo hijoForward = current;
            if (!HayObstaculoDelante(hijoForward.st)){
                if (Cerrados.find(hijoForward.st) == Cerrados.end()){
                    hijoForward.secuencia.push_back(actFORWARD);
                    Abiertos.push(hijoForward);
                }
            }
		}

		// Tomo el siguiente valor de la Abiertos
		if (!Abiertos.empty()){
			current = Abiertos.front();
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);
		return true;
	}
	else {
		cout << "No encontrado plan\n";
	}


	return false;
}

// Implementación de la busqueda por menor coste.
// Entran los puntos origen y destino y devuelve la
// secuencia de acciones en plan, una lista de acciones.
bool ComportamientoJugador::pathFinding_CosteUniforme(const estado &origen, const estado &destino, list<Action> &plan){
    //Borro la lista
	cout << "Calculando plan\n";
    plan.clear();

    set<estado,ComparaEstados> explorados;                  // Nodos ya explorados (cerrados)
	priority_queue<nodo, vector<nodo>, ComparaCoste> cola;  // Lista de nodos por explorar
	unsigned char casilla = '?';    // valor random

    // Inicializar
    nodo current;
	current.st = origen;
	current.secuencia.empty();
	current.coste = 0;

	cola.push(current);

    while (!cola.empty() and (current.st.fila != destino.fila or current.st.columna != destino.columna)){
        cola.pop();
        casilla = mapaResultado[current.st.fila][current.st.columna]; // Guardamos la casilla actual

        if(explorados.find(current.st) == explorados.end()){ // optimización
            explorados.insert(current.st);  // insertar

            // Generar desdendiente de girar a la derecha
            nodo hijoTurnR = current;
            hijoTurnR.st.orientacion = (hijoTurnR.st.orientacion+1)%4;

            if (explorados.find(hijoTurnR.st) == explorados.end()){ // Si el nodo no ha sido explorado
                hijoTurnR.coste += getCosteGiro(hijoTurnR.st);      // calculamos el coste de moverse a él
                hijoTurnR.secuencia.push_back(actTURN_R);
                cola.push(hijoTurnR);
            }

            // Generar descendiente de girar a la izquierda
            nodo hijoTurnL = current;
            hijoTurnL.st.orientacion = (hijoTurnL.st.orientacion+3)%4;

            if (explorados.find(hijoTurnL.st) == explorados.end()){ // Si el nodo no ha sido explorado
                hijoTurnL.coste += getCosteGiro(hijoTurnL.st);
                hijoTurnL.secuencia.push_back(actTURN_L);
                cola.push(hijoTurnL);
            }

            // Generar desdendiente de avanzar
            nodo hijoForward = current;

            if (!HayObstaculoDelante(hijoForward.st)){
                if (explorados.find(hijoForward.st) == explorados.end()){ // Si no lo encuentra
                    hijoForward.secuencia.push_back(actFORWARD);
                    hijoForward.coste = current.coste += getCoste(hijoForward.st); // Calculamos coste de la casilla
                    cola.push(hijoForward);
                }
            }

        }

		// Siguiente valor en cola
		if (!cola.empty()){
			current = cola.top();

			// Comprobar si tenemos bikini o zapatillas
            if(mapaResultado[current.st.fila][current.st.columna] == 'K' && !current.st.bikini){
                current.st.bikini = true;
                current.st.zapatillas = false;
            } else if (mapaResultado[current.st.fila][current.st.columna] == 'D' && !current.st.zapatillas){
                current.st.bikini = false;
                current.st.zapatillas = true;
            }
		}
	}

  cout << "Terminada la busqueda\n";

	if (current.st.fila == destino.fila and current.st.columna == destino.columna){
		cout << "Cargando el plan\n";
		plan = current.secuencia;
		cout << "Longitud del plan: " << plan.size() << endl;
		PintaPlan(plan);
		// ver el plan en el mapa
		VisualizaPlan(origen, plan);

		return true;
	}
	else {
		cout << "No se ha encontrado un plan\n";
	}

	return false;
}


// Sacar por la consola la secuencia del plan obtenido
void ComportamientoJugador::PintaPlan(list<Action> plan) {
	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			cout << "A ";
		}
		else if (*it == actTURN_R){
			cout << "D ";
		}
		else if (*it == actTURN_L){
			cout << "I ";
		}
		else {
			cout << "- ";
		}
		it++;
	}
	cout << endl;
}


// Funcion auxiliar para poner a 0 todas las casillas de una matriz
void AnularMatriz(vector<vector<unsigned char> > &m){
	for (int i=0; i<m[0].size(); i++){
		for (int j=0; j<m.size(); j++){
			m[i][j]=0;
		}
	}
}


// Pinta sobre el mapa del juego el plan obtenido
void ComportamientoJugador::VisualizaPlan(const estado &st, const list<Action> &plan){
  AnularMatriz(mapaConPlan);
	estado cst = st;

	auto it = plan.begin();
	while (it!=plan.end()){
		if (*it == actFORWARD){
			switch (cst.orientacion) {
				case 0: cst.fila--; break;
				case 1: cst.columna++; break;
				case 2: cst.fila++; break;
				case 3: cst.columna--; break;
			}
			mapaConPlan[cst.fila][cst.columna]=1;
		}
		else if (*it == actTURN_R){
			cst.orientacion = (cst.orientacion+1)%4;
		}
		else {
			cst.orientacion = (cst.orientacion+3)%4;
		}
		it++;
	}
}



int ComportamientoJugador::interact(Action accion, int valor){
  return false;
}

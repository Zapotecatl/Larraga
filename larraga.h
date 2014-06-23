#ifndef LARRAGA_H
#define LARRAGA_H


/* Extern "C" */
//#ifdef __cplusplus
//extern "C" {
//#endif

#include <stdio.h>
#include <iostream>
#include <algorithm>
#include <vector>


#include <QDebug>

using namespace std;

struct Color {
    int r;
    int g;
    int b;
};

struct SVehicle {

    int position;
    int velocity;
    Color color;

};



int myrandom (int i);
int CityBuilding(int l);
void printHighway();
void CityDestroying();
void SwitchHighwayRW();
int CalcularTablas();
void printTable();
void SetValueCellStreet(int x, int value);
int GetValueCellStreet(int x);
void SetPositionVehicle(int id, int pos);
void SetVelocityVehicle(int id, int vel);
int GetPositionVehicle(int id);
int GetVelocityVehicle(int id);
int distanceN(int n);
int DistribuiteVehicles(float density);
void InicilizationCity(int l, float d);
void DestroyingAll();

int CalcularRa();
struct Color GetColorVehicle(int id);
int mainFunction();
void printDistances();
void printPositions();
double frand();
int RunSimulation(int tick);



void Velocity();
void Flux();
void SaveMeasures();

/* Extern "C" */
//#ifdef __cplusplus
//}
//#endif

#endif // LARRAGA_H







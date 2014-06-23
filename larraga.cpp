/*////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Description: This program implements in QT the cellular automaton model for traffic flow based on safe driving policies and human reactions proposed
by M. E. Larraga and L. Alvarez-Icaza.

Autor: Jorge Luis Zapotecatl López

Date: 03/23/2014

*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#include "larraga.h"

vector<int> random_pos;


int total_cell;
float velocity;
float flux;

float velocity_total;
float flux_total;
int n_ticks = 2 * 1800; //Los primeros 1800 para estabilizar el sistema, los siguientes 1800 son evaluados

float densitiy = 0.3; //Usado en los experiemntos
int length_h = 10000; //Longitud de la carretera (son 10000 en el paper) (usado en los experimentos)
int size_vehices; //Numero de vehiculos
float delta_x = 5;
float delta_v = 5 / delta_x; //cambie la formula para ajustar a una celda de la longitud del vehiculo
int vmax = 1; //velocidad maxima
int M = 2;// Capacidad de frenado (5.00m / Dx = 2.5m)
float *Ra; //Probabilidad de desacelerar
float Rs = 0.001;
float R0 = 0.8;
float Rd = 1.0;
int ls = 1;//Longitud de los vehiculos
int vs = (5 / 5) + 1; //Velocidad baja

bool switch_highway; //Controla el cambio de apuntadores de escirtura y lectura
bool switch_vehicles;

SVehicle *pr_vehicles;
SVehicle *pw_vehicles;
SVehicle *vehicles_A;
SVehicle *vehicles_Z;

int *pr_highway; //Apuntador a arreglo de lectura
int *pw_highway; //Apuntador a arrego de escritura
int *highway_A;
int *highway_Z;

unsigned int **acc;
unsigned int **keep;
unsigned int **dec0;

FILE *fp_f;
FILE *fp_v;


// random generator function:
int myrandom (int i)
{
    return rand() % i;
}

int myrandomBetween (int min, int max)
{
    int randNum;

    return  randNum = rand() % (max - min + 1) + min;
}

double frand()
{
    return (double)rand() / RAND_MAX;
}

//The city is set to agree to the number of horizontal and vertical streets, and the distance of the blocks
int CityBuilding(int l)
{
    int i;

    //Sets values ​​valid in the numbers of horizontal and vertical streets, and the distance of the blocks.
    if (l < 1)
        length_h = 1;
    else
        length_h = l;// L

    highway_A = new int[length_h];
    highway_Z = new int[length_h];

    SwitchHighwayRW();

    //The streets are divided into cells. The state of the cells are classified as follows:
    //0 - The cell is empty
    //1 - The cell contains a vehicle
    for (i = 0; i < length_h; i++){
        highway_A[i] = 0;
        highway_Z[i] = 0;
    }

    //printHighway();
    return 0;
}

void printHighway()
{
    int i;

    printf ("---Highway----\n");
    for (i = 0; i < length_h; i++)
        printf ("%d ", GetValueCellStreet(i));
    printf ("\n");

    /*
    printf ("---Write----\n");
    for (i = 0; i < length_h; i++)
        printf ("%d ", pw_highway[i]);
    printf ("\n");
    */

}

void CityDestroying()
{
    //int i;

    delete [] highway_A;
    delete [] highway_Z;

    pr_highway = NULL;
    pw_highway = NULL;

}

void TablasRaDestroying()
{
    int i;

    for (i = 0; i < vmax + 1; i++)
        delete acc[i];
    delete [] acc;

    for (i = 0; i < vmax + 1; i++)
        delete keep[i];
    delete [] keep;

    for (i = 0; i < vmax + 1; i++)
        delete dec0[i];
    delete [] dec0;

    delete [] Ra;

}

void SwitchHighwayRW()
{

    if (switch_highway == true){
        pr_highway = &highway_A[0];
        pw_highway = &highway_Z[0];
    }
    else {
        pr_highway = &highway_Z[0];
        pw_highway = &highway_A[0];
    }

    switch_highway = !switch_highway;
}

void SwitchVehiclesRW()
{

    if (switch_vehicles == true){

        pr_vehicles = &vehicles_A[0];
        pw_vehicles = &vehicles_Z[0];
    }
    else {

        pr_vehicles = &vehicles_Z[0];
        pw_vehicles = &vehicles_A[0];
    }

    switch_vehicles = !switch_vehicles;
}

void InicilizationCity(int l, float d)
{

    switch_highway = true;
    switch_vehicles = true;

    CityBuilding(l);
    DistribuiteVehicles(d);
    CalcularTablas();//calcular las distancias fuera de linea
    CalcularRa();//calcular probabilidad Ra fuera de linea

}

int CalcularRa()
{
    int i;

    Ra = new float[vmax + 1];
    for (i = 0; i < vmax + 1; i++)
        Ra[i] = min(Rd, R0 + (float)i * ((Rd - R0) / (float)vs));

   /* printf ("Ra: ");
    for (i = 0; i < vmax + 1; i++)
        printf ("%f ", Ra[i]);*/

}

int CalcularTablas()
{

    int i;
    int dnacc, dnkeep, dndec;
    int dp = 0;

    dnacc = dnkeep = dndec = 0;

    acc = new unsigned int*[vmax + 1];
    for (i = 0; i < vmax + 1; i++)
        acc[i] = new unsigned int[vmax + 1];

    keep = new unsigned int*[vmax + 1];
    for (i = 0; i < vmax + 1; i++)
        keep[i] = new unsigned int[vmax + 1];

    dec0 = new unsigned int*[vmax + 1];
    for (i = 0; i < vmax + 1; i++)
        dec0[i] = new unsigned int[vmax + 1];

    int vn = 0, vp = 0;

    for (vn = 0; vn <= vmax; vn++) {

        for (i = 0; i <= (vn + 1) / M; i++)
            dnacc += (vn + 1) - (i * M);

        for (i = 0; i <= (vn) / M; i++)
            dnkeep += (vn - i * M);

        for (i = 0; i <= ((vn - 1) / M); i++)
            dndec += (vn - 1) - (i * M);

        dndec = (dndec < 0) ? 0 : dndec;
        dnkeep = (dnkeep < 0) ? 0 : dnkeep;
        dnacc = (dnacc < 0) ? 0 : dnacc;

        for (vp = 0; vp <= vmax; vp++) {

            for (i = 0; i <= (vp - M) / M; i++)
                dp += (vp - M) - (i * M);

            dp = (dp < 0) ? 0 : dp;

            acc[vn][vp] = ((dnacc - dp) < 0) ? 0 : dnacc - dp;
            keep[vn][vp] = ((dnkeep - dp) < 0) ? 0 : dnkeep - dp;
            dec0[vn][vp] = ((dndec - dp) < 0) ? 0 : dndec - dp;
            dp = 0;
        }

        dnacc = dnkeep = dndec = 0;
    }

    return 0;

}

void printTable()
{

    int i, j;

    printf("----Acelerar----\n");
    for (i = 0; i <= vmax; i++) {
        for (j = 0; j <= vmax; j++)
            printf("%d   ", acc[i][j]);
        printf("\n");
    }

    printf("----Mantener----\n");
    for (i = 0; i <= vmax; i++) {
        for (j = 0; j <= vmax; j++)
            printf("%d   ", keep[i][j]);
        printf("\n");
    }

    printf("----Desacelerar----\n");
    for (i = 0; i <= vmax; i++) {
        for (j = 0; j <= vmax; j++)
            printf("%d   ", dec0[i][j]);
        printf("\n");
    }

}

void SetValueCellStreet(int x, int value)
{
    if (x < 0 || x >= length_h)
       x = 0;

    pw_highway[x] = value;

}

void SetPositionVehicle(int id, int pos)
{

    if (id < 0 || id >= size_vehices)
        id = 0;

    pw_vehicles[id].position = pos;
}

void SetVelocityVehicle(int id, int vel)
{

    if (id < 0 || id >= size_vehices)
        id = 0;

    pw_vehicles[id].velocity = vel;

}

void SetColorVehicle(int id, struct Color color)
{

    if (id < 0 || id >= size_vehices)
        id = 0;

    pw_vehicles[id].color = color;

}

struct Color GetColorVehicle(int id)
{

    if (id < 0 || id >= size_vehices)
        id = 0;

    return pr_vehicles[id].color;

}


int GetValueCellStreet(int x)
{
    int value;

    if (x < 0 || x >= length_h)
       x = 0;

    value = pr_highway[x];

    return value;
}

int GetPositionVehicle(int id)
{

    int value;

    if (id < 0 || id >= size_vehices)
        id = 0;

    value = pr_vehicles[id].position;

   return value;

}

int GetVelocityVehicle(int id)
{

    int value;

    if (id < 0 || id >= size_vehices)
        id = 0;

    value = pr_vehicles[id].velocity;

   return value;

}

int distanceN(int n)
{
    int d;
    int n1;


    if (n < 0 || n >= size_vehices)
        n = 0;

    if (n == (size_vehices - 1))
        n1 = 0;
    else
        n1 = n + 1;

    if (GetPositionVehicle(n) == GetPositionVehicle(n1))
        printf ("ERROR: No puede haber dos vehiculos en la misma celda");

    if (GetPositionVehicle(n) < GetPositionVehicle(n1)){

        d = (GetPositionVehicle(n1) - GetPositionVehicle(n)) - 1;
        //qDebug() << "C1N: " << n << "pn1 - pn - 1: " << GetPositionVehicle(n1) << " - " << GetPositionVehicle(n) << " = " << d;
    }
    else {

        d = (length_h + GetPositionVehicle(n1)) - GetPositionVehicle(n) - 1;
        //qDebug() << "C2P: " << n << "d = (l - 1) - pn - p0" << n << " : " << length_h - 1 << " - " << GetPositionVehicle(n) << " + " << GetPositionVehicle(n1) << " = " << d;
    }

    return d;
}

int DistribuiteVehicles(float density)
{

    int i;

    for (i = 0; i < length_h; i++)
       random_pos.push_back(i);

    random_shuffle(random_pos.begin(), random_pos.end(), myrandom);//Coloca en ordena aleatorio las posiciones iniciales

    if (density < 0)
        density = 0;
    else if (density > 1)
        density = 1;

    size_vehices = density * random_pos.size();

    vehicles_A = new SVehicle[size_vehices];
    vehicles_Z = new SVehicle[size_vehices];

    SwitchVehiclesRW();

    random_pos.resize(size_vehices);
    sort(random_pos.begin(), random_pos.end());

    i = 0;
    while (i < size_vehices) {

        SetPositionVehicle(i, random_pos[i]);
        //SetVelocityVehicle(i, myrandom(vmax + 1)); //Causa que los vehiculos colisionen (ocupar mismas celdas) el AC porque la velocidad es muy grande...
        SetVelocityVehicle(i, 0);
        SetValueCellStreet(random_pos[i], i); //se pone a i porque es considerado como un id, posteriormente es necesario para identificar el vehiculo al pintarlo en una interfaz grafica

        struct Color color;

        color.r = myrandomBetween (127, 255);
        color.g = myrandomBetween (0, 127);
        color.b = myrandomBetween (127, 255);

        //SetColorVehicle(i, color);
        //Hay que escribir al inicio en los dos arreglos, color es una propiedad constante
        vehicles_A[i].color = color;
        vehicles_Z[i].color = color;

        //qDebug() << "R: " << color.r;
        //qDebug() << "G: " << color.g;
        //qDebug() << "B: " << color.b;

        i++;
    }

    random_pos.clear();
    SwitchHighwayRW();
    SwitchVehiclesRW();

    return 0;
}

void printDistances()
{
    int i;

    printf ("-------------distancias-------------\n");
    printf ("Length: %d\n", length_h);
    for (i = 0; i < size_vehices; i++)
        printf ("distance %d - %d\n", i + 1, distanceN(i));
}

void printPositions()
{
    int i;

    printf("---------------Positions---------------\n");

    for (i = 0; i < size_vehices; i++)
            printf("%d  ", GetPositionVehicle(i));
    printf("\n");

}

int RunSimulation(int tick)
{
    //Parametros efectivos vmax ,ls ,Rd ,Ra y M

    int i;
    unsigned int dn;
    int xn, vn;
    int vn_new;
    int vp;


     for (i = 0; i < length_h; i++)
        SetValueCellStreet(i, -1);//clear the lattice of write

    vn_new = 0;

    for (i = 0; i < size_vehices; i++) {

        dn = distanceN(i);

        //qDebug() << "A-Vehiculo: " << i << " P: " << GetPositionVehicle(i) << " V: " << GetVelocityVehicle(i) << "D:" << dn;

        if (i == (size_vehices - 1)) {
            xn = GetPositionVehicle(i);
            vn = GetVelocityVehicle(i);
            vp = GetVelocityVehicle(0);

        }
        else {
            xn = GetPositionVehicle(i);
            vn = GetVelocityVehicle(i);
            vp = GetVelocityVehicle(i + 1);
        }

        //Acceleration
        if (dn >= acc[vn][vp]){
            if (frand() <= Ra[vn]){
              int tmp_vn = vn + delta_v;
              vn_new = min(tmp_vn, vmax);
            }
            else
                vn_new = vn;
        }
        else if (dn < acc[vn][vp] && dn >= keep[vn][vp] ){//Random slowing
            if (frand() <= Rs){
                int tmp_vn = vn - delta_v;
                vn_new = max(tmp_vn, 0);
            }
            else
                vn_new = vn;
        }
        else if (dn < keep[vn][vp] && dn >= dec0[vn][vp] && vn > 0){//breaking
            int tmp_vn = vn - delta_v;
            vn_new = max(tmp_vn, 0);
        }
        else if (dn < dec0[vn][vp] && vn > 0){//emergency breaking
            int tmp_vn = vn - M;
            vn_new = max(tmp_vn, 0);
        }

        if ((xn + vn_new) >= length_h)
           xn = (xn + vn_new) % length_h;
        else
           xn = xn + vn_new;

        SetPositionVehicle(i, xn);
        SetVelocityVehicle(i, vn_new);
        SetValueCellStreet(xn, i);//se pone a i porque es considerado como un id, posteriormente es necesario para identificar el vehiculo al pintarlo en una interfaz grafica

        //qDebug() << "D-Vehiculo: " << i << " P: " << xn << " V: " << vn_new;

    }

    if (tick > (n_ticks / 2.0)) {//relaxation 10 * 10^4
        Velocity();
        Flux();
    }

    SwitchHighwayRW();
    SwitchVehiclesRW();

    return 0;
}



//Measures////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Velocity()
{

    int i;

    float vel = 0;
    for (i = 0; i < size_vehices; i++)
        vel+= GetVelocityVehicle(i);

    int n_vmax = vmax * size_vehices; //para normalizar los valores

    velocity = vel / n_vmax;
    velocity_total+= velocity;

}

void Flux()
{
    //qDebug() << "Flujo" << ((float) densitiy * velocity);
    flux = ((float) densitiy * velocity);
    flux_total+= flux;
}


void SaveMeasures()
{

    velocity_total = velocity_total / (n_ticks / 2.0);
    flux_total = flux_total / (n_ticks / 2.0);

    //fprintf(fp, "%f\t%f\t%f\n", densitiy, velocity_total, flux_total);
    fprintf(fp_v, "%f %f\n", densitiy, velocity_total);//con estos datos se grafica el diagrama fundamental del trafico
    fprintf(fp_f, "%f %f\n", densitiy, flux_total);//con estos datos se grafica el diagrama fundamental del trafico
}

int mainFunction()
{

    printf ("Start...\n ");
    clock_t start = clock();

    fp_f = fopen("measuresflu.txt", "w");   // Abrir archivo para escritura
    fp_v = fopen("measuresvel.txt", "w");   // Abrir archivo para escritura

    for (densitiy = 0.02; densitiy <= 1.0; densitiy+=0.02){

        //printf("%f\n", densitiy);

        switch_highway = true;
        switch_vehicles = true;

        velocity = 0;
        flux = 0;
        velocity_total = 0;
        flux_total = 0;

        InicilizationCity(length_h, densitiy);

        int tick;
        for (tick = 0; tick < n_ticks; tick++)
            RunSimulation(tick);


        SaveMeasures();

        //printf("Density: %f", densitiy);
        qDebug() << densitiy;
        DestroyingAll();
    }

    //printf ("End...\n ");
    printf("Tiempo transcurrido: %f", ((double)clock() - start) / CLOCKS_PER_SEC);
    fclose(fp_f);
    fclose(fp_v);
}


void DestroyingAll()
{

    TablasRaDestroying();
    CityDestroying();

    delete [] vehicles_A;
    delete [] vehicles_Z;

}



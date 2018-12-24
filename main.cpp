#include <stdio.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>
#include <string.h>
#include <ctime>
#include <time.h>
#include <iterator>
#include <math.h>
#include <vector>

#include "Splitter.h"

using namespace std;


const char * filename_FlowTraffic = "stream.dat";

char insert[N_INSERT + N_INSERT / 5][200];
char **query;

unordered_map<string, int> unmp;
unordered_map<string, int> unmp1;


int main(int argc, char** argv)
{
    if(argc == 2)
    {
        filename_FlowTraffic = argv[1];
    }

	unmp.clear();
    unmp1.clear();
	double val;


    double memory = 0.05;  //MB

    int w = memory * 1024 * 1024 * 8.0 / COUNTER_SIZE;
    printf("error_allowed = %f\n", tau*WINDOW_SIZE/((double)w / (double)LOW_HASH_NUM));


    printf("\n******************************************************************************\n\n");
    printf("Evaluation starts!\n\n");


    Splitter *splitter;

    int packageNum = 0;

 	FILE *file_FlowTraffic = fopen(filename_FlowTraffic, "r");

    while(fgets(insert[packageNum], 200, file_FlowTraffic) != NULL)
    {
        unmp[string(insert[packageNum])]++;

        packageNum++;

        if(packageNum == N_INSERT)
            break;
    }
    fclose(file_FlowTraffic);


    for (int i = 7000; i < 10000; i++)
    {
        unmp1[string(insert[i])]++;
    }

    query = new char*[unmp.size()];
    for(int i = 0; i < unmp.size(); ++i)
        query[i] = new char[200];


    int k = 0, max_freq = 0;
    for(unordered_map<string, int>::iterator it = unmp.begin(); it != unmp.end(); it++, k++)
    {
        strcpy(query[k], (it->first).c_str());

        int tmpVal = it->second;
        max_freq = max_freq > tmpVal ? max_freq : tmpVal;
    }
    printf("max_freq = %d\n", max_freq);


    splitter = new Splitter(w / LOW_HASH_NUM, LOW_HASH_NUM);
    for(int i = 0; i < 10000; i++)
    {
        splitter->Insert(insert[i], i);
    }
    printf("packageNum = %d\n", packageNum);


    double re_cm = 0.0;
    double re_cm_sum = 0.0;
    double ae_cm = 0.0;
    double ae_cm_sum = 0.0;
	double val_cm = 0.0;    
    int cm_cor_num = 00;

    char temp[200];

    int under_E = 0;
    int over_E = 0;
    for(unordered_map<string, int>::iterator it = unmp1.begin(); it != unmp1.end(); it++)
    {
        strcpy(temp, (it->first).c_str());
    	val = (double)it->second;

		val_cm = splitter->Query(temp, 9999);
        if (val_cm < val)
        {
            under_E++;
        }
        if (val_cm > val)
        {
            over_E++;
        }

        re_cm = fabs(val_cm - val) / (val * 1.0);
        ae_cm = fabs(val_cm - val);
        re_cm_sum += re_cm;
        ae_cm_sum += ae_cm;

        if(val == val_cm)
            cm_cor_num++;
    }

    double extra;
    extra = (double)splitter->memoryusage / 8 / 1024 / 1024;
    printf("sketch memoryusage is : %f\n", memory);
    printf("extra memoryusage is : %f\n\n", extra);

    double a = unmp1.size() * 1.0;
    double b = packageNum;

    printf("under_E = : %d\n", under_E);
    printf("over_E = : %d\n\n\n", over_E);
    printf("cm\tcorrect rate = %lf\n", cm_cor_num * 1.0 / a);
    printf("-------------------------------------\n\n");

    printf("are_cm = %lf\n", re_cm_sum / a);
    printf("-------------------------------------\n\n");

    printf("aae_cm = %lf\n", ae_cm_sum / a);
    printf("-------------------------------------\n\n");


    printf("Evaluation Ends!\n");
    printf("******************************************************************************\n\n");

    return 0;
}

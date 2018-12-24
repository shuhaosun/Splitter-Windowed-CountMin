#ifndef _CMSKETCH_H
#define _CMSKETCH_H

#include <algorithm>
#include <cstring>
#include <string.h>
#include "params.h"
#include "BOBHash32.h"
#include <iostream>
#include <stdlib.h>

using namespace std;

typedef struct Node
{
	double init;
	double last;
	double count;
	struct Node *next;
	struct Node *prior;
}DLNode;

class Splitter
{	
private:
	BOBHash32 * bobhash32[MAX_HASH_NUM];
	int index[MAX_HASH_NUM];
	double *counter[MAX_HASH_NUM];
	int w, d;
	int MAX_CNT;
	int counter_index_size;
	uint64_t hash_value;
	DLNode *head[N_INSERT];


public:
	int mem_acc;
	int memoryusage = 0;
	Splitter(int _w, int _d)
	{
		mem_acc = 0;
		counter_index_size = 20;
		w = _w;
		d = _d;
		
		for(int i = 0; i < d; i++)	
		{
			counter[i] = new double[w];
			memset(counter[i], 0, sizeof(double) * w);
		}

		MAX_CNT = (1 << COUNTER_SIZE) - 1;

		for(int i = 0; i < d; i++)
		{
			bobhash32[i] = new BOBHash32(i + 1000);
		}
	}
	void ListInitiate(DLNode **head)
	{
		*head = (DLNode *)malloc(sizeof(DLNode));
		(*head)->prior = *head;
		(*head)->next = *head;
	}
	int ListInsert(DLNode *head, int a, int b, int x)
	{
		DLNode *p,*s;
		p = head->prior;

		s = (DLNode *)malloc(sizeof(DLNode));
		s->init = a;
		s->last = b;
		s->count = x;

		s->prior = p;
		s->next = p->next;
		p->next->prior = s;
		p->next = s;
//		printf("p->next->init= %f\n", p->next->init);
//		printf("p->next->last= %f\n", p->next->last);
		memoryusage = memoryusage + 2*12 + 16;
	}
	double Error(DLNode *p, DLNode *l)
	{
		double error;
		double Fprev = p->count / (l->init - p->init);
		double Fnow = l->count / (l->last - l->init + 1);
		if (Fprev > Fnow) 
			error = Fprev / Fnow;
		else 
			error = Fnow / Fprev;

		if (error == 0)
		{
			printf("error = : %f\n", error);
		}
		return error;
	}
	void Merge(DLNode *head, int t)
	{
		DLNode *p,*l;
		l = head->prior;
		p = l->prior;
		p->count = p->count + l->count;
        p->last = l->last;
        l->count = 1;
        l->init = t;
        l->last = t;
	}
	void Expire(DLNode *head, int t, int i, int j)
	{
		DLNode *p, *q;
		double ratio;
		double scale;

		p = head->next;

		while(p != head && ((p->init < t - WINDOW_SIZE) || (p->init == t - WINDOW_SIZE)))
		{
			if (p->last > t - WINDOW_SIZE)//the second node is totally in the window
			{
				ratio = (t - WINDOW_SIZE) - p->init + 1;
				scale = p->count / (p->last - p->init + 1);
				counter[i][j] = counter[i][j] - ratio*scale;

				p->count = p->count - ratio*scale;
				p->init = p->init + ratio;
				p = p->next;
			}
			else
			{
				counter[i][j] = counter[i][j] - p->count;

				//delete p node
				q = p;
				p = p->next;
			    q->prior->next = q->next;
				q->next->prior = q->prior;
				free(q);
				if (p == head)
				{
					counter[i][j] = 0;
				}
				memoryusage = memoryusage - 2*12 - 16;
			}
		}
	}

	void Insert(const char * str, int t)
	{
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash32[i]->run(str, strlen(str))) % w;
			if((int)counter[i][index[i]] != MAX_CNT)
			{
				int ij = w*i + index[i];
				if(counter[i][index[i]] == 0)
				{
					ListInitiate(&head[ij]);
				}
				else if ( (head[ij]->next->init < t - WINDOW_SIZE) || (head[ij]->next->init == t - WINDOW_SIZE))
				{
					Expire(head[ij], t, i, index[i]);
				}

				if (counter[i][index[i]] < 0)
				{
					counter[i][index[i]] = 0;
				}
				counter[i][index[i]]++;
				if (counter[i][index[i]] == 1)
				{
					ListInsert(head[ij], t, t, 1);
				}
				else if (head[ij]->prior->count < tau*WINDOW_SIZE/(double)w) 
				{
					head[ij]->prior->count++;
					head[ij]->prior->last = t;
				}
				else if (head[ij]->prior->prior != head[ij] && (Error(head[ij]->prior->prior, head[ij]->prior) < mu || Error(head[ij]->prior->prior, head[ij]->prior) == mu))
				{
					Merge(head[ij], t);
				}
				else
				{
					ListInsert(head[ij], t, t, 1);
				}
			}
			mem_acc ++;
		}
	}
	double Query(const char *str, int t)
	{
		double min_value = MAX_CNT;
		double temp;
		for(int i = 0; i < d; i++)
		{
			index[i] = (bobhash32[i]->run(str, strlen(str))) % w;
			int ij = w*i + index[i];
			Expire(head[ij], t, i, index[i]);

			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}
	
	~Splitter()
	{
		for(int i = 0; i < d; i++)	
		{
			delete []counter[i];
		}


		for(int i = 0; i < d; i++)
		{
			delete bobhash32[i];
		}
	}
};
#endif//_CMSKETCH_H

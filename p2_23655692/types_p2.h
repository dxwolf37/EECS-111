#ifndef __TYPES_P2_H
#define __TYPES_P2_H

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/time.h>
#include <string>
#include <vector>
#include <unistd.h>
using namespace std;

#define EMPTY        0
#define WOMENPRESENT 1
#define MENPRESENT   2


class Person
{

	int gender; // 0: male 1: female
	std::string str_gender;
	struct timeval t_create;
	struct timeval t_start;
	struct timeval t_end;
	long time_to_stay_ms;


	unsigned long order;
	unsigned long use_order;

public:
	Person();

	void set_gender(int data);
	int get_gender(void);

	void set_order(unsigned long data);
	unsigned long get_order(void);

	void set_use_order(unsigned long data);
	unsigned long get_use_order(void);

	void set_time(long data);
	int ready_to_leave(void);

	void start(void);
	void complete(void);
	unsigned long acquire_time(void);
};


// Class for the restroom
// You may need to add more class member variables and functions
class Restroom {
	int status, num, sum, tallyMen, tallyWomen, alternate;

	// You need to define the data structure to
    // save the information of people using the restroom
	// You can probebly use Standard Template Library (STL) vector


public:
	Restroom(){
		status = EMPTY;
		alternate, sum, tallyMen, tallyWomen = 0;
	}
	vector<Person> q_vector; 
	vector<Person> rr_vector;
	
	void set_input(int data);
	int get_input(void);
	// You need to use this function to print the Restroom's status
	void print_status(void);
	void set_status(int data);
	int get_status(void);


	// Call by reference
	// This is just an example. You can implement any function you need
	void add_person(Person& p);
	void remove_person(int a);
	void man_wants_to_enter(Person& p);
	void woman_wants_to_enter(Person& p);
	void woman_leaves(Person& p);
	void man_leaves(Person& p);
	void status_switched(Person& p);
};










#endif

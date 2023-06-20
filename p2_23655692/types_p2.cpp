#include "types_p2.h"
#include "utils.h"

void Restroom::set_input(int data){num = data;}
int Restroom::get_input(void) {return num;}
void Restroom::set_status(int data){status = data;}
int Restroom::get_status(void){return status;}

void Person::set_gender(int data) { gender = data; }
int Person::get_gender(void)      { return gender; }

void Person::set_order(unsigned long data) { order = data; }
unsigned long Person::get_order(void)      { return order; }

void Person::set_use_order(unsigned long data) { use_order = data; }
unsigned long Person::get_use_order(void)      { return use_order; }

void Person::set_time(long data) { time_to_stay_ms = data; }
unsigned long Person::acquire_time(void){return time_to_stay_ms;}
int Person::ready_to_leave(void) {
	struct timeval t_curr;
	gettimeofday(&t_curr, NULL);

	if (get_elasped_time(t_start, t_curr) >= time_to_stay_ms) { return 1; }
	else { return 0; }
}



void Person::start(void) {
	gettimeofday(&t_start, NULL);
	printf("(%lu)th person enters the restroom: \n", order);
	printf(" - (%lu) milliseconds after the creation\n", get_elasped_time(t_create, t_start));
}

void Person::complete(void) {
	gettimeofday(&t_end, NULL);
	printf("(%lu)th person comes out of the restroom: \n", order);
	printf(" - (%lu) milliseconds after the creation\n", get_elasped_time(t_create, t_end));
	printf(" - (%lu) milliseconds after using the restroom\n", get_elasped_time(t_start, t_end));
}

Person::Person() {
	gettimeofday(&t_create, NULL);
}

// You need to use this function to print the Restroom's status
void Restroom::print_status(void) {
	if (rr_vector.size() == 0 && status == 0){
		cout<<"(Empty): Total: "<<sum<<" (Men: "<<tallyMen<<", Women: "<<tallyWomen<<")"<<endl;
	}
	else if (rr_vector.size() == 0 && status == 1){
		cout<<"(Empty): Total: "<<sum<<" (Men: "<<tallyMen<<", Women: "<<tallyWomen<<")"<<endl;
	}
	else if (rr_vector.size() == 0 && status == 2){
		cout<<"(Empty): Total: "<<sum<<" (Men: "<<tallyMen<<", Women: "<<tallyWomen<<")"<<endl;
	}
	else if (status == 1){
		cout<<"(WomanPresent): Total: "<<sum<<" (Men: "<<tallyMen<<", Women: "<<tallyWomen<<")"<<endl;
	}
	else{
		cout<<"(MenPresent): Total: "<<sum<<" (Men: "<<tallyMen<<", Women: "<<tallyWomen<<")"<<endl;
	}
}

// Call by reference
// This is just an example. You can implement any function you need
void Restroom::add_person(Person& p) {
	int size;
	rr_vector.push_back(p);
	size = rr_vector.size() - 1;
	rr_vector[size].start();
	sum = sum + 1;
	tallyMen = tallyMen + (p.get_gender() == 0);
	tallyWomen = tallyWomen + (p.get_gender() == 1);
}

void Restroom::status_switched(Person& p){
	int temp = status;
	status = (rr_vector.size() == 0) ? 0 : (p.get_gender() == 0) ? 2 : 1;
	alternate = (status == temp) ? 0 : 1;
}

void Restroom::remove_person(int a){
	int gender = rr_vector[a].get_gender();
	status_switched(rr_vector[a]);
	sum = sum - 1;
	tallyMen = tallyMen - (gender == 0);
	tallyWomen = tallyWomen - (gender == 1);
	rr_vector.erase(a + rr_vector.begin());
	Person reg = rr_vector[a];

	if (gender == 0) {
    	man_leaves(reg);
	} else {
    	woman_leaves(reg);
	}

	if (rr_vector.empty()) {
    	set_status(0);
	}
}

void Restroom::woman_leaves(Person& p){
	bool isZero = (rr_vector.size() == 0);
	cout << "[Restroom] (Woman) left the restroom. Status is ";
	if (isZero) {
    	cout << "changed, ";
	}
	print_status();
}

void Restroom::woman_wants_to_enter(Person& p){
	bool sendWoman = (p.get_gender() == 1 && (status == 1 || (status == 2 && q_vector.empty())));
	cout << "[Queue] Send (Woman) into the restroom (Stay " << p.acquire_time() << "ms), Status:";
	if (sendWoman)
    	cout << " ";
	print_status();
	add_person(q_vector[0]);
}

void Restroom::man_leaves(Person& p){
	bool isZero = (rr_vector.size() == 0);
	cout << "[Restroom] (Man) left the restroom. Status is ";
	if (isZero) {
	    cout << "changed, ";
	}
	print_status();
}

void Restroom::man_wants_to_enter(Person& p){
	bool sendMan = (p.get_gender() == 0 && (status == 2 || (status == 1 && q_vector.empty())));
	cout << "[Queue] Send (Man) into the restroom (Stay " << p.acquire_time() << "ms), Status:";
	if (sendMan)
 	   cout << " ";
	print_status();
	add_person(q_vector[0]);
}



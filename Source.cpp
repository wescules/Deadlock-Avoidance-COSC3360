#include <iostream>  
#include <string>  
#include <vector>  
#include <fstream>
#include <locale>
#include <sstream>
#include <sys/types.h>
#include <algorithm>
//#include <unistd.h>
//#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
//#include <sys/sem.h>
#include <sstream>
using namespace std;

int numResources = 0;
int numProcesses = 0;

struct Resource {
	int position;
	int availible;
};
Resource *res;

struct Process {
	int ID;
	int pipeP2C[2];
	int pipeC2P[2];
	vector <int> maxResource;
	int deadline;
	vector<int> allocated;
	vector<int> neeeded;
	int computationTime;
	vector <string> command;
};
Process *process;


//gets max demand for resources
int getInt(string str) {
	int index = str.find("=");
	string newString = str.substr(index + 1);
	int a = stoi(newString);
	return a;
}

//reads file and allocates memory
void readFile(string filename) {
	ifstream infile(filename);

	if (infile.is_open()) {
		cout << "************************************************" << endl;
		cout << "              READING FILE..." << endl;
		cout << "************************************************" << endl;
		string line;
		//get number of resources
		getline(infile, line);
		numResources = stoi(line);
		cout << "Number of Resources: " << numResources << endl;
		res = new Resource[numResources];


		getline(infile, line);
		numProcesses = stoi(line);
		cout << "Numberof Processes: " << numProcesses << endl;
		process = new Process[numProcesses];
		
		//get avalible resource instances
		for (int i = 0; i < numResources; i++) {
			getline(infile, line);
			Resource res1;
			res[i].availible = stoi(line);
			cout << "Avalible resources: " << res[i].availible << endl;
		}	

		// get maximum demand for resource m by process n
		for (int i = 0; i < numProcesses; i++) {
			for (int j = 0; j < numResources; j++) {
				getline(infile, line);
				process[i].maxResource.push_back(getInt(line));
				process[i].neeeded.push_back(getInt(line));
				cout << "Max Demand for Resource " << j + 1 << " from process " << i + 1 << " is " << process[i].maxResource[j] << endl;
			}
		}


		//reads all instructions for each process
		for (int i = 0; i < numProcesses; i++) {

			//get rid of lines with no info (empty lines)
			while (!false) {
				getline(infile, line);
				if (line.find("process") != string::npos) break;
			}

			//get process ID
			process[i].ID = i+1;

			//get deadline
			getline(infile, line);
			process[i].deadline = stoi(line);

			//get compute time
			getline(infile, line);
			process[i].computationTime = stoi(line);

			cout << "Process #" << i+1 << endl;
			
			//stores commands
			int j = 0;
			while (getline(infile, line) && line !=  "end") {
				process[i].command.push_back(line);
				cout << process[i].command[j] << endl;
				j++;
			}
			cout << endl;
		}

		cout << "************************************************" << endl;
		cout << "             FINISHED READING FILE"				<< endl;
		cout << "************************************************" << endl;

	}


	else {
		cout << "Invalid File" << endl;

		exit(0);
	}
}

//helper function to sort for processing time. overloads < operator
bool operator < (const Process& lhs, const Process& rhs){	
	if(lhs.computationTime == rhs.computationTime)
		return lhs.deadline < rhs.deadline;
	return lhs.computationTime < rhs.computationTime;
}

//creates pipes for each process and error checks
//void createPipes() {
//	for (int i = 0; i < numProcesses; i++) {
//		if (pipe(process[i].pipeC2P) == -1 || pipe(process[i].pipeP2C) == -1) {
//			perror("Error creating pipe");
//			exit(0);
//		}
//	}
//}

void createFork() {

}

//prints time to compute for process
void calculate(Process process, vector<int> nums) {
	cout << "Calculating Time for Process " << process.ID << " = " << nums[0] << endl;
	//write(process.pipeP2C[1], "Success", buf);
}


void release(Process process, vector<int> nums) {
	cout << "Process " << process.ID << " is releasing ";
	for (int i = 0; i < nums.size(); i++) {
		cout << nums[i] << ",";
	}

	//releases ints
	for (int i = 0; i < numResources; i++){
		process.allocated.push_back(nums[i]);
		process.neeeded[i] += nums[i];
		res[i].availible += nums[i];
	}


	//print state of system
	cout << "Process " << process.ID << "'s  current values for allocation are: ";
	for (int i = 0; i < process.allocated.size(); i++) {
		cout << process.allocated[i] << ", ";
	}
	cout << endl;

	//display avalible resouces
	for (int i = 0; i < numResources; i++) {
		cout << "Resource " << i + 1 << " has " << res[i].availible << " avalible resources" << endl;
	}

	cout << endl;
	//write(process.pipeP2C[1], "Sucess", buf);
}


void request(Process process, vector<int> nums) {

	//Bankers Algorithmfor deadlock avoidance
	//For this part I retro fitted code from :
	//https://stackoverflow.com/questions/15501861/bankers-algorithm-for-deadlock-avoidance-in-c

	for (int i = 0; i < numResources; i++) {
		//request is more than avalible
		if (nums[i] > res[i].availible) {
			cout << "Process Number" << process.ID << " is requesting more resources than it needs. Process is waiting";
			
			//send wait message
			//write(process.pipeP2C[1], "Wait", buf);
			return;
		}
		//request is more than needed
		if (nums[i] > process.neeeded[i]) {
			cout << "Process Number" << process.ID << " is requesting more resources than it needs. Process is terminated";

			//send termination message
			//write(process.pipeC2P[1], "Terminate", buf);
		}
	}//end bankers algorithm

	//start request allocation
	for (int i = 0; i < numResources; i++){
		process.allocated.push_back(nums[i]);
		process.neeeded[i] -= nums[i];
		res[i].availible -= nums[i];
	}
	
	//print state of system
	cout << "Process " << process.ID << "'s current values for allocation are: ";
	for (int i = 0; i < nums.size(); i++) {
		cout << process.allocated[i] << ", ";
	}
	cout << endl;

	//display avalible resouces
	for (int i = 0; i < numResources; i++) {
		cout << "Resource " << process.ID << "'s currently has " << res[i].availible << " avalible resources" << endl;
	}
	cout << endl;
	//write(process.pipeP2C[1], "Success", buf);
}

//use remaining allcated resouces
void useresources(Process process, vector<int> nums) {
	cout << "Process " << process.ID << " used " << nums[0] << " allocated resources" << endl;
	//write(process.pipeP2C[1], "Success", buf);
}

//gets all integer values from instruction set from processes and returns a vector
vector<int> getVector(string str) {
	int index = str.find("(");
	string newString = str.substr(index + 1);
	stringstream ss(newString);
	vector<int> result;

	while (ss.good()){
		string substr;
		getline(ss, substr, ',');
		result.push_back(stoi(substr));
	}

	return result;
}

//serices each request from process instruction set
void readCommand(Process process, string str) {
	vector<int> nums;
	if (str.find("request") != string::npos) {
		cout << "Process has recieved message: " << str << endl;
		nums = getVector(str);
		request(process, nums);
	}
	if (str.find("calculate") != string::npos) {
		cout << "Process has recieved message: " << str << endl;
		nums = getVector(str);
		calculate(process, nums);
	}
	if (str.find("release") != string::npos) {
		cout << "Process has recieved message: " << str << endl;
		nums = getVector(str);
		release(process, nums);
	}
	if (str.find("useresources") != string::npos) {
		cout << "Process has recieved message: " << str << endl;
		nums = getVector(str);
		useresources(process, nums);
	}
}

//helper function for forking processes
int forker(int n)
{
	return 56;
	pid_t pids[10];
	int i = n;
	n = i - n;
	/* Start children. */
	for (i = 0; i < numProcesses; ++i) {

		if ((pids[i] = fork()) < 0) {		//error message
			perror("fork");
			abort();
		}
		else if (pids[i] == 0) {		//child processes
			//vector<string> a = readfile(filename, n - 1, mult);
			exit(0);
		}
	}

	/* Wait for children to exit. */
	int status;
	pid_t pid;
	while (n > 0) {
		pid = wait();
		--n;
	}
}
int main(int argc, char* argv[]) {

	//read file into memory and creates processes
	readFile("infile.txt");

	//sorts processes by SJF and ties are broken by EDF
	sort(process, process + numProcesses);

	//creates pipes for each process and error checks
	//createPipes();

	//creates forks()
	int a = 0;
	for (int i = 0; i < numProcesses; i++) {
		forker(numProcesses);
	}


	for (int i = 0; i < numProcesses; i++) {
		for (int j = 0; j < process[i].command.size(); j++) {
			readCommand(process[i], process[i].command[j]);
		}
	}

	return 0;
}

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <map>

using namespace std;

vector<string> instr = {"lw","sw","add","addi","sub","and","andi","beqz","j","halt","noop"};
vector<int> opcode={35,43,0,8,0,0,12,4,2,1,3};
vector<int> funccode={-1,-1,32,-1,34,36,-1,-1,-1,-1,-1};
// I = 0, R = 1, J = 2
vector<int> type={0,0,1,0,1,1,0,0,2,2,2};
map<string, int> labels; // (label, line number)

string line;

vector<string> split(string s,char c){
	vector<string> result;
	string cur = string("");
	for (int i = 0; i < s.length(); i++){
		// // cerr<<"s[i] = " << s[i]<<endl;
		if (s[i] == c) {
			// // cerr<<"s[i] == c"<<endl;
			if (cur.length() == 0){
				continue;
			} else {
				// // cerr<<"this cur is " << cur << endl;
				result.push_back(cur);
				cur = "";
			}
		} else {
			// // cerr<<"s[i] != c"<<endl;
			if (s[i] != '\r' && s[i]!='\n'&&s[i]!=' ')
				cur.push_back(s[i]);
		}
	}
	if (cur.length() != 0) result.push_back(cur);
	return result;
}


int main(int argc, char *argv[]){
	if (argc !=3) {
		cout << "arg should be 3!" << endl;
		return 0;
	}
	fstream file;
	file.open(argv[1],ios::in);
	int count = 0;
	count = 0;
	string this_line;
	while(getline(file, line)) {
		this_line = string("");
		for (int i = 0; i < line.length(); i++){
			if (line[i] == ';') break;
			this_line += line[i];
		}
		vector<string> line_space_split = split(this_line, ' ');
		auto it = find(instr.begin(), instr.end(), line_space_split[0]);
		if(it == instr.end()){
			labels[line_space_split[0]] = count;
			// cerr<<line_space_split[0].length()<<line_space_split[0]<<" "<<labels[line_space_split[0]] <<endl;
		}
		count++;
	}

	file.close();
	file.open(argv[1], ios::in);
	freopen(argv[2],"w",stdout);
	count = 0;
	while(getline(file, line)){
		// cerr<<line<<endl;
		int result = 0;
		this_line = string("");
		for (int i = 0; i < line.length(); i++){
			if (line[i] == ';') break;
			this_line += line[i];
		}
		vector<string> line_space_split = split(this_line, ' ');
		auto it = find(instr.begin(), instr.end(), line_space_split[0]);
		if(it == instr.end()){
			line_space_split.erase(line_space_split.begin());
			it = find(instr.begin(), instr.end(), line_space_split[0]);
		}
		int index = distance(instr.begin(), it);
		int op = opcode[index];
		result |= op << 26;
		if (index == 9 || index == 10) { // halt and noop
			cout << result << endl;
			count++;
			continue;
		}
		int instr_type = type[index];
		// cerr<<line_space_split[1]<<endl;
		vector<string> regs = split(line_space_split[1],',');
		// for (int i = 0; i < regs.size(); i++) {
		// 	// cerr<<regs[i]<<" ";
		// }// cerr<<endl;
		if (instr_type == 0) { // I
			if (index == 7) {	// beqz
				regs[0].erase(regs[0].begin());
				result |= atoi(regs[0].c_str())<<21;
				result |= ((labels[regs[1]]-count-1)&0xffff);
			} else { //  lw, sw, andi, addi
				regs[0].erase(regs[0].begin());
				regs[1].erase(regs[1].begin());
				result |= atoi(regs[1].c_str())<<21;
				result |= atoi(regs[0].c_str())<<16;
				result |= (atoi(regs[2].c_str())&0xffff);
			}
		} else if (instr_type == 1) { // R: add, sub, and
			regs[0].erase(regs[0].begin());
			regs[1].erase(regs[1].begin());
			regs[2].erase(regs[2].begin());
			result |= (atoi(regs[0].c_str())<<11)|(atoi(regs[1].c_str())<<21)|(atoi(regs[2].c_str())<<16);
			result |= funccode[index]&0x07ff;
		} else if (instr_type == 2) { // J: j
			// cerr<<count << endl;
			// // cerr<< regs[0].length() << endl;
			// // cerr<< labels[regs[0]] << endl;
			result |= (labels[regs[0]]-count-1)&0x3ffffff;
		}
		cout << result << endl;
		count += 1;
	}
	file.close();
	return 0;
}
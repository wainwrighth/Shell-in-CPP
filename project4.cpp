// Names: Harrison Wainwright
// CS 485
// 11/6/16
// Project 4

#include <string>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <unistd.h>
#include <stdlib.h>
#include <wait.h>
#include <cstring>
#include <fcntl.h>
#include <fstream>

using namespace std;

// List of functions to be used.
void scanner(vector <string>& com, bool file, string line);
void tokenize(vector <string>& com, vector <string>& tok);
void execCommand(vector <string>& com, vector <string>& tok);
void execCD(vector <string>& com, vector <string>& tok);
void execSet(vector <string>& com, vector <string>& tok);
void execRun(vector <string>& com, vector <string>& tok);
void execAssign(vector <string>& com, vector <string>& tok);
string parse(string command, int pos);
void runFork(const char* path, vector <const char*> arguments, bool bg);
void fillArg(vector <string>& com, vector <const char*>& arg);
void assignFork(const char* path, vector <const char*> arguments);

// Initialize vectors for commands and tokens to be used later.
vector <string> commands;
vector <string> tokens;
vector <string> paths;
vector <pid_t> processes;
string prompt = "sish > ";
int showTokens = 0;

int main(int argc, char** argv)
{
	ifstream in_file;
	string line;
	bool fileRead = false;

	paths.push_back("PATH=/bin:/usr/bin");

	// If file was added to program run read it and run them.
	if(argc == 2)
	{
		fileRead = true;

		in_file.open(argv[1]);

		if(in_file.fail())
		{
			cout << "file was not found" << endl;
			return 2;
		}
		
		while(getline(in_file, line))
		{				
			tokens.clear();
			commands.clear();

			scanner(commands, fileRead, line);

		        // If user enters 'done' as their first command, continue to skip other code and allow proper exiting.
               		if(commands[0] == "done")
              		{
                       		return 0;
                	}

               		// A percent was entered first as a comma, continue and get next input.
               		if(commands[0] == "skip")
                	{
                	       continue;
                	}
			
			// If user doesn't enter 'done' or an invalid keyword, tokenize the values of the commands.
                	tokenize(commands, tokens);

                	if(tokens[0] != "keyword")
                	{
                	        cout << "invalid command" << endl;
				continue;
                	}

                	if(showTokens == 1)
                	{
                	        for(int i = 0; i < commands.size(); i++)
                	        {
                	                cout << "Token = " << commands[i] << endl;
                	        }
               		}

               		// Value entered is a keyword, now check for proper token count.
               		execCommand(commands, tokens);
		}

		in_file.close();
		cout << endl;
		return 0;
	}
	else if(argc > 2)
	{
		cout << "incorrect argument count was entered." << endl;
		return 1;
	}
	else
	{
		do
		{
			// Clear the two vectors in case of previous filling from old input.
			tokens.clear();
			commands.clear();
	
			// Call scanner fucntion to get input and fill commands vector.
			scanner(commands, fileRead, "");
	
			// If user enters 'done' as their first command, continue to skip other code and allow proper exiting.
			if(commands[0] == "done")
			{
				continue;
			}

			// A percent was entered first as a comma, continue and get next input.
			if(commands[0] == "skip")
			{
				continue;
			}
	
			// If user doesn't enter 'done' or an invalid keyword, tokenize the values of the commands.
			tokenize(commands, tokens);
	
			if(tokens[0] != "keyword")
			{
				cout << "invalid command" << endl;
				continue;
			}
	
			if(showTokens == 1)
			{
				for(int i = 0; i < commands.size(); i++)
				{
					cout << "Token = " << commands[i] << endl;
				}
			}

			// Value entered is a keyword, now check for proper token count.
			execCommand(commands, tokens);

		}while(commands[0] != "done"); // Keep getting user input until the "done" command is entered.
	
		// Exit with status zero.
		return 0;
	}
}

// Purpose: To get user input and parse the values into an array for interpretation.
// Preconditions: A command vector that is empty initially.
// Postconditions: A command vector that is filled with commands enetered by user.
void scanner(vector <string>&  com, bool file, string line)
{
	string userInput;
	string subStr;
	int foundPos;

	cout << prompt;

	if(file)
	{
		userInput = line;
	}
	else
	{
		getline(cin, userInput);
	}

	static const size_t npos = -1;
	userInput += " ";
	string delim = " ";

	// Check for 'Ctrl-D' input by user and account for it.
	if(!cin.good())
	{
		cout << endl;
		exit(0);
	}

	// If user entered a comment, skip over it.
	if(userInput[0] == '%')
	{
		com.push_back("skip");
		return;
	}

	if(userInput.find(delim) == npos)
	{
		com.push_back(userInput);
	}

	while(userInput.find(delim) != npos)
	{
		if(userInput[0] == ' ')
                {
                        userInput = userInput.substr(1);
                        continue;
                }

		subStr = userInput.substr(0, userInput.find(delim));

		if(subStr[0] == '"')
		{
			foundPos = userInput.find('"', 1);
			subStr = userInput.substr(0, foundPos + 1);
			com.push_back(subStr);
                	userInput = userInput.substr(foundPos + 1);
			continue;
		}

		com.push_back(subStr);
		userInput = userInput.substr(userInput.find(delim) + 1);
	}
}

// Purpose: To properly fill the tokens vector based on the commands vector that has been filled.
// Preconditions: The commands vector and the empty tokens vector.
// Postconditions: A properly filled tokens vector.
void tokenize(vector <string>& com, vector <string>& tok)
{
	string temp;

	// If the first command entered is 'done' return to prevent further token
	if(com[0] == "done")
	{
		return;
	}

	for(int i = 0; i < com.size(); i++)
	{
		temp = com[i];

		if(temp == "run" || temp == "assignto" || temp == "cd" || temp == "set" || temp == "defprompt" || temp == "listprocs")
		{
			tok.push_back("keyword");
		}
		else if(temp[0] == '"')
		{
			tok.push_back("string");
		}
		else if(temp == "%" || temp == "&")
		{
			tok.push_back("metaChar");
		}
		else if(temp[0] == '$')
		{
			tok.push_back("variable");
		}
		else
		{
			tok.push_back("word");
		}
	}
}

// Goes through the first value of the com vector and runs the proper command.
void execCommand(vector <string>& com, vector <string>& tok)
{
        if(com[0] == "run")
        {
		if(com.size() == 1)
		{
			com[0] = "skip";
		}
		else
		{
			execRun(com, tok);
		}
	}
        else if(com[0] == "assignto")
        {
		if(com.size() <= 2)
		{
			cout << "Insufficient parameters" << endl;
			com[0] = "skip";
		}
		else
		{
			execAssign(com, tok);
		}
        }
        else if(com[0] == "cd") // If user entered command 'cd', check token count.
        {
		if(com.size() != 2) // User entered incorrect token count.
		{
			cout << "expected 2 tokens, got " << com.size() << " tokens." << endl;
		}
		else
		{
			execCD(com, tok); // Call execCD to execute cd.
		}
        }
        else if(com[0] == "set") // If user entered command 'set', check token count.
        {
		if(com.size() != 3) // User entered incorrect token count.
		{
			cout << "expected 3 tokens, got " << com.size() << " tokens." << endl;
		}
		else
		{
			execSet(com, tok); // Call execSet to execute set.
		}	
        }
        else if(com[0] == "defprompt") // If user entered command 'defprompt', check token count and execute.
        {
		if(com.size() != 2) // User entered incorrect number of tokens.
		{
			cout << "expected 2 tokens, got " << com.size() << " tokens." << endl;
		}
		else // Execute 'defprompt'.
		{
			prompt = com[1];
		}
        }
        else if(com[0] == "listprocs")
        {
		if(com.size() != 1)
		{
			com[0] = "skip";
		}
		else
		{
       			cout << "listprocs command" << endl;
		
			if(processes.size() != 0)
			{
				cout << "Background processes:" << endl;

				for(int i = 0; i < processes.size(); i++)
				{
					cout << processes[i] << endl;
				}
			}
			else
			{
				cout << "no background processes" << endl;
			}	
		}
        }
}

// Purpose: To change directory to the one given by the user.
// Preconditions: The commands and tokens vectors.
// Postconditions: A changed directory if successful and a message of failure if needed.
void execCD(vector <string>& com, vector <string>& tok)
{
	// Initialize values and set directory constant character value.
	string var = com[1];
	string del = "=";
	const char* directory = com[1].c_str();

	// If the second parameter is led by a '$', get the path value.
	if(var[0] == '$')
	{
		// Go through paths vector and search for path name.
		for(int i = 0; i < paths.size(); i++)
		{
			// If path name is a match, save its right side into directory.
			if(paths[i].substr(0, paths[i].find(del)) == com[1].substr(1))
			{
				directory = (paths[i].substr(paths[i].find(del) + 1)).c_str();
			}
		}
	}

	// Change directory to given directory name and note if doing so fails.
	if(chdir(directory) != 0)
	{
		cout << directory << ": No such file or directory" << endl;
	}
}

// Purpose: Execute proper code based on correct 'set' keyword and parameter count.
// Preconditions: Vectors for commands and tokens.
// Postconditions: A new ShowTokens value or a new path saved in the paths vector.
void execSet(vector <string>& com, vector <string>& tok)
{
        if(com[1] == "ShowTokens") // If user eneters "ShowTokens" check next value.
	{
		if(com[2] == "1" || com[2] == "\"1\"") // Set value of showTokens to 1.
		{
			showTokens = 1;
		}
		else if(com[2] == "0" || com[2] == "\"0\"") // Set value of showTokens to 0.
		{
			showTokens = 0;
		}

		return;
	}
	else // 2 parameters are given, so check if there is a word and a string.
	{
		if(tok[1] == "word" && tok[2] == "string")
		{

			// Pull quotes from string value and store it in paths vector.
			string newStr = com[2];
			newStr = newStr.substr(1, newStr.length() - 2);

			// Check to see if path name already exists in paths vector,
			for(int i = 0; i < paths.size(); i++)
			{
				string del = "=";
				
				if(paths[i].substr(0, paths[i].find(del)) == com[1]) // If path name exists replace it.
				{
					paths[i] = com[1] + "=" + newStr; // It exists already, change path value.
					return;
				}
			}		

			paths.push_back(com[1] + "=" + newStr); // Otherwise path is new, add it to paths vector.
		}
	}
}

// Run the run command.
void execRun(vector <string>& com, vector <string>& tok)
{
	vector <const char*> args;
	const char* cmd = com[1].c_str();
	string path = com[1];
	char directory[256];
	bool background = false;
	const char* endPathChar;
	string pathName;

	// If user enters cat and a variable starting with '$' print the associated value.
	if(com[1] == "cat" && com[2].substr(0, 1) == "$")
	{
		for(int i = 0; i < paths.size(); i++)
		{
			if((paths[i].substr(0, paths[i].find("="))) == com[2].substr(1))
			{
				cout << paths[i].substr(paths[i].find("=") + 1) << endl;
				return;
			}
		}

		// Value was not found, inform the directory or filename was not found.
		cout << com[2] << ": No such file or directory" << endl;
	}

	// If command ends with '&' change bool to run it in the background.
	if(com[com.size() - 1] == "&")
	{
		com.pop_back();
		background = true;
	}

	// Get proper directory and fill arguments vector then execute run.
	if(cmd[0] == '.' && cmd[1] == '/')
        {
		pathName = path.substr(1);
		
		string endPath = getcwd(directory, sizeof(directory));
		endPath += pathName;

		endPathChar = endPath.c_str();

		args.push_back(endPathChar);
		
	        if(com.size() > 2)
        	{
                	fillArg(com, args);
        	}

		args.push_back(0);

		runFork(endPathChar, args, background);
        }
        else if(cmd[0] == '/')
        {
		args.push_back(cmd);

                if(com.size() > 2)
                {
                        fillArg(com, args);
                }

                args.push_back(0);

		runFork(cmd, args, background);
        }
	else
	{
		pathName = "/bin/" + path;
		endPathChar = pathName.c_str();
		
                args.push_back(endPathChar);

                if(com.size() > 2)
                {
                        fillArg(com, args);
                }

                args.push_back(0);

		runFork(endPathChar, args, background);
	}
}

// run the assignto command.
void execAssign(vector <string>& com, vector <string>& tok)
{
	vector <const char*> args;
        const char* endPathChar;
        string pathName;
        string path = com[2];
	char directory[256];
	bool assignSuccess = false;
	string del = "=";
	string line;
	ifstream file;

	string dir = com[2];

	// If '&' was entered, tell user and return.
	for(int i = 0; i < com.size(); i++)
	{
		if(com[i] == "&")
		{
			cout << "assignto does not accept the argument '&'" << endl;
			com[0] = "skip";
			return;
		}
	}
	
	// Get proper directory and fill args vector and run assignto exec.
	if(dir[0] == '/')
	{
		assignSuccess = true;

		args.push_back(dir.c_str());
		fillArg(com, args);
		args.push_back(0);

		assignFork(dir.c_str(), args);
	}
	else if(dir[0] == '.' && dir[1] == '/')
	{
		assignSuccess = true;

                pathName = path.substr(1);

                string endPath = getcwd(directory, sizeof(directory));
                endPath += pathName;

                endPathChar = endPath.c_str();

                args.push_back(endPathChar);
                fillArg(com, args);
		args.push_back(0);

		assignFork(endPathChar, args);
	}
	else
	{
		assignSuccess = true;

		pathName = "/bin/" + path;
                endPathChar = pathName.c_str();

                args.push_back(endPathChar);
		fillArg(com, args);
		args.push_back(0);

                assignFork(endPathChar, args);
	}

	// After assignFork is done giving values to the tempfile, assign them.
	if(assignSuccess)
	{
		string var = com[1];
		string pathVal;
		string fileVals;

		// Check for pre-existance and overwrite if so.
		for(int i = 0; i < paths.size(); i++)
		{
			if((paths[i].substr(0, paths[i].find(del))) == commands[1])
			{
				file.open("./tempfile");
					
				while(getline(file, line))
				{
					fileVals += line;
					fileVals += '\n';
				}

				fileVals = fileVals.substr(0, fileVals.length() - 1);

				paths[i] = paths[i].substr(0, paths[i].find(del)) + "=" + fileVals;

				cout << paths[i];
					
				file.close();
				remove("./tempfile");
				return;
			}
		}

		// Add in path and associated value to paths vector.
		pathVal = com[1];
		file.open("./tempfile");

		while(getline(file, line))
		{
			fileVals += line;
			fileVals += '\n';
		}

		fileVals = fileVals.substr(0, fileVals.length() - 1);

		string input = pathVal + "=" + fileVals;
		paths.push_back(input);

		file.close();
		remove("./tempfile");
	}
}

// get proper value on right side of '=' from paths vector.
string parse(string command, int pos)
{
	string del = "=";

	for(int i = 0; i < paths.size(); i++)
	{
		if((paths[i].substr(0, paths[i].find(del))) == commands[pos].substr(1))
		{
			return command = (paths[i].substr(paths[i].find(del) + 1));
		}
	}
	
	return command;
}

// run run command with forking and execv calls.
void runFork(const char* path, vector <const char*> arguments, bool bg)
{
	int childStatus;
	pid_t pid;

	pid = fork();

	if(pid < 0) // Failed
	{
		return;
	}
	else if(pid == 0) // Run child process.
	{
		execv(path, (char**)&arguments[0]);// call exec for child process.
		
		exit(0);// The child failed to exec, return.
	}
	else
	{
		if(bg == false)
		{
			wait(&childStatus); // Parent will wait for the completion of the child process.
		}
		else
		{
			processes.push_back(pid);
		}
	}

	return;
}

// Fill args vector with proper arguments.
void fillArg(vector <string>& com, vector <const char*>& arg)
{
	const char* argument;
	int type;

	if(com[0] == "run")
	{
		type = 2;
	}
	else if(com[0] == "assignto")
	{
		type = 3;
	}

	// Put all of the parameters in an argument vector 'args'.
        for(int i = type; i < com.size(); i++)
        {
                string val = com[i];

                if(val[0] == '$')
                {
			argument = (parse(val, i)).c_str();
                }
                else
                {
                        argument = val.c_str();
		}

                arg.push_back(argument);
         }
}

// Run assignto by forking and creating temporary file then using execv call.
void assignFork(const char* path, vector <const char*> arguments)
{
	int childStatus;
	pid_t pid;
	const char* fileName = "./tempfile";
	int fileDesc;

	pid = fork();

	if(pid < 0)
	{
		return;
	}
	else if(pid == 0)
	{
		// Use creat to make the file with proper read and write permissions
		fileDesc = creat(fileName, S_IRUSR|S_IWUSR);
		dup2(fileDesc, 1);
		close(fileDesc);

		execv(path, (char**)&arguments[0]);// call exec for child process.

                exit(0); // The child failed to exec, return.
	}
	else
	{
		wait(&childStatus); // Parent will wait for the completion of the child process.
	}

	return;
}

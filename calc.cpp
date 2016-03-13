#include "std_lib_facilities.h"

/*
Simple calculator
Ephraim Raj Feb 16 2016

This program implements 
basic expression calculator.

Imput from cin, output out of cout.

Grammar of input is 
Calculate :
	Statement  
	Print
	Quit
	Help
Print :
	newline
	;
Quit :
	quit
Help
	help
Statement :
	Declaration
	Expression
Declaration :
	let name = expression
	const name = expression
Expression :
	Assignment
	Term
	Expression + Term
	Expression - Term
	Expression print Expression  
Assignment : 
	Name = expression
Term :
	Primary
	Term * Primary
	Term / Primary
	Term % Primary
Primary :
	+Primary
	-Primary
	(Expression)
	{Expression}
	Name
	Number
Number : 
	Floating-point literal
Name :
	string literal
Input comes from a Token_stream called ts.

*/

// *** MAGIC CONSTANTS *** \\

//keywords
const string quitkey = "quit";
const string helpkey = "help";
const string conskey = "const";
const string declkey = "let";

//token.kind values
const char quit = 'q';
const char help = 'h';
const char result_sym = '=';
const char print = ';';
const char prompt = '>';
const char number = '8';
const char name = 'a';
const char let = 'L';
const char cons = 'c';

//  *** HELP MESSAGE ** \\ 

void help_message(){
	cout << "This is a calculator which includes variable support";
	cout << "\nEvaluated in the order of BODMAS";
	cout << "\nSupported symbols : + - * / % () {} ";
	cout << "\nExample expression : 5*{2+(3-2)}/8; ";
	cout << "\nDeclare variables like : let var_name = expression;";
	cout << "\nExample : let a = 5;";
	cout << "\nAssign already declared variables as : var_name = expression;";
	cout << "\nExample : a = 3 + 2;";
	cout << "\nUse const instead of let while declaring to prevent assignment later";
	cout << "\nExample : const tau = 6.28;";
	cout << "\npi and e are already declared constants";
	cout << "\nPress ; to end statement";
	cout << "\nType in \"help\" for this help screen";
	cout << "\nType in \"quit\" to quit\n";

}


// *** TOKEN, TOKEN_STREAM AND THEIR FUNCTIONS *** \\

class Token {
public:
	char kind;
	double value;
	string name;
	Token(char ch) :kind{ch} { }                                
    Token(char ch, double val) :kind{ch}, value{val} { }                                                                                                     
    Token(char ch, string n) :kind{ch}, name{n} { }            
                                                                                                         
};

class Token_stream {
public:
	void ignore(char c); // ignore till ch
	Token_stream(); 
	Token get(); // get from Token_stream
	void putback(Token t); // putback into Token_stream
private:
	bool full = false;
	Token buffer;
};

void Token_stream::ignore(char c){
	if(full && buffer.kind == c){
		full = false;
		return;
	}
	full = false;
	char ch = 0;
	while(cin.get(ch)){
		if(ch == c || ch == '\n')
			return;
	}
}

Token_stream::Token_stream()
:full(false), buffer(0)    // no Token in buffer
{
}

void Token_stream::putback(Token t){
	if(full)
		error("Filled buffer");
	buffer = t;
	full = true;
	
}

Token Token_stream::get(){
	if(full){ 
		full = false;
		return buffer; // buffer doesnt get deleted!
	}

	char ch;
	cin.get(ch); // read white space too
	
	while(isspace(ch)){ // if newline, print; if other whitespace, take next char
		if(ch == '\n')	// till we get a non white space
			return Token{print};
		else 
			cin.get(ch);
	}
	
	switch(ch){
		case print : 
		case '=' :
		case '(' : 
		case ')' :
		case '{' :
		case '}' :
		case '+' :
		case '-' :
		case '*' :
		case '/' :
		case '%' :
		return Token{ch};
		case '.' : case '0' : case '1' : case '2' : case '3' : case '4' : 
		case '5' : case '6' : case '7' : case '8' : case '9' :
		{
			cin.putback(ch);
			double val;
			cin >> val;
			return Token{number, val};
		}
		default : 
		if(isalpha(ch) || ch == '_'){ // first character of name condition
			string s;
			s+= ch;
			while(cin.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_'))
				s+=ch; // other characters of name
			cin.putback(ch); // putback left out invalid character
			// keywords
			if(s == declkey) return Token (let);
			if(s == conskey) return Token (cons);
			if(s == helpkey) return Token (help);
			if(s == quitkey) return Token (quit);
			// end keywords
			return Token{name, s}; // returns a name with type name
		}
		error("Invalid Token");
	}
}



// *** VARIABLES *** \\

class Variable{
public: 
	string name;
	double val;
	bool is_const; // is a constant
	Variable(string n, double val, bool t) :name{n}, val{val}, is_const{t}  { }
};

class Symbol_table{ // functions operating on the variable table
public: 
	double get(string s); // gets the value of given name
	void set(string s, double val); //sets value to already declared variable
	double declare(string s, double val, bool is_const ); 
private:
	vector<Variable> var_table; // holds the variables
	bool is_declared(string s); // check if a variable with name s is declared
};

bool Symbol_table::is_declared(string s){ // check if already declared
	for(const Variable& x : var_table)
		if(x.name == s) 
			return true;
	return false;
}

double Symbol_table::declare(string s, double val, bool is_const = false){
	if(is_declared(s)) error("Already declared ", s);
	if(is_const)
		var_table.push_back(Variable{s,val,true});
	else
		var_table.push_back(Variable{s,val,false});
	return val;
}

double Symbol_table::get(string s){
	for(const Variable& x : var_table){
		if(x.name == s) return x.val;
	}
	error("get_value() cannot find ", s);
	
}

void Symbol_table::set(string s, double val){ 
	for(Variable& x : var_table)
		if(x.name == s){
			if(x.is_const == true) error("Is a constant"); 
			x.val = val;
			return;
			}
	
	error("set_value() cannot find ", s);

}

Symbol_table vars; 



// *** MATHEMATICAL EXPRESSION GRAMMAR *** \\
double expression(); // so primary can use it
double primary(){
	Token t = ts.get();

	switch(t.kind){
		case '(' :
				{
					double in_brack = expression();
					t = ts.get();
					if(t.kind != ')')
						error("No ')' to complete");
					return in_brack;
				}
		case '{' :
				{
					double in_brack = expression();
					t = ts.get();
					if(t.kind != '}')
						error("No '}' to complete");
					return in_brack;
				}
		case '+' : return primary();
		case '-' : return - primary();
		case number : return t.value;
		case name : {
			Token next = ts.get();
			if(next.kind == '=') { // if its an assignment
				double val = expression();
				vars.set(t.name, val);
				return val;
			}
			else
					{ts.putback(next); // not an assignment
				return vars.get(t.name);}
		}
		default : error("Primary expected");
	}
}

double term() {
	double left = primary();
	Token t = ts.get();

	while(true){
		switch(t.kind){
			case '*' : left*= primary();
						t = ts.get(); break;
			case '/' : {
						double den = primary();
						if(den == 0) error("Divide by zero");
						left/= den; 
						t = ts.get();
						break;
					}
			case '%' : {
						double den = primary();
						if(den == 0) error("Divide by zero");
						left = fmod(left, den);
						t = ts.get();
						break;
					}
			default : ts.putback(t);
						return left;
		}
	}
}


double expression(){
	double left = term();
	Token t = ts.get();

	while(true){
		switch(t.kind){
			case '+' : left+= term(); 
					   t = ts.get();
					   break;
			case '-' : left-= term(); 
					   t = ts.get();
					   break;
			default : ts.putback(t); 
					return left; 

		}
	}
}
void clean_up_mess(){// you know, to clean up mess
	
		ts.ignore(print); // ignore till print
	
}

double declaration(Token k){
	Token t = ts.get();
	if(t.kind!= name) 
		error("Name expected");
	string var_name = t.name;
	Token t2 = ts.get();
	if(t2.kind != '=')
		error("= expected");
	double val = expression();
	if(k.kind == let)
		vars.declare(t.name, val);
	else if (k.kind = cons)
		vars.declare(t.name, val, true);
	return val;
}


double statement(){
	Token t = ts.get();
	switch(t.kind){
		case cons : 
		case let : return declaration(t.kind);
		default : ts.putback(t); return expression();
	}
}

void calculate(){
	while(cin)
		try {  	
  		cout << prompt;
    	Token t = ts.get();
    	while(t.kind == print) t=ts.get(); // eat token
    	if(t.kind == quit)
    		return;
    	if(t.kind == help){
    		help_message();
    		continue;
    	} 

    	ts.putback(t);
    	cout << result_sym << statement() << "\n";
	}
	catch(exception& e){
		cerr << "Error : " << e.what() << "\n";
		clean_up_mess();
	}
}


int main()
try { 
	vars.declare("pi",3.1415926535, true);
    vars.declare("e",2.7182818284, true);
	calculate();
	cout << '\n';
	return 0;
}
catch(exception& e){
  cerr << "Error : " << e.what() << '\n';
  cout << "Enter ~ to exit. \n"; // So that left out stream 
  for (char ch; cin >> ch;)      // dont do the press key to exit 
  		if(ch == '~') return 1;  // for us
  return 1;
}
catch(...){
  cerr << "Error : Unknown exception";
  return 2;
}




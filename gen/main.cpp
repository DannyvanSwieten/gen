//
//  main.cpp
//  gen
//
//  Created by Danny on 20/03/2018.
//  Copyright © 2018 Danny. All rights reserved.
//

#include <cassert>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

const std::string space 		= " ";
const std::string assign 		= " = ";
const std::string semi_colon 	= ";";
const std::string const_float 	= "const float ";
const std::string acc_open		= "{";
const std::string acc_close		= "}";
const std::string tab			= "\t";

class Context {
public:
	
	static Context& get() {
		static Context c;
		return c;
	}
	
	std::string getUniqueName(const std::string& original) {
		if(names.find(original) != names.end())
			return original;
		
		std::string unique = original + "_" + std::to_string(version++);
		names.emplace(unique);
		return unique;
	}
	
	void createGlobals(std::stringstream& stream) {
		for(const auto& name: names)
			stream << "float " << name << semi_colon << std::endl;;
	}
	
private:
	Context() = default;
	std::set<std::string> names;
	
	std::size_t version = 0;
};

struct Layout {
	size_t indentLevel = 0;
	
	std::string getWhiteSpace() {
		std::string ws;
		for(auto i = 0; i < indentLevel; i++)
			ws += tab;
		
		return ws;
	}
};

struct Node {
	Node(const std::string& result): result(Context::get().getUniqueName(result)) { }
	virtual void generate(Layout& layout, std::stringstream& stream) { };
	
	const std::string result;
	bool generated = false;
};

struct Constant: public Node {
	Constant(const std::string& name, float value): Node(name), value(value) { };
	void generate(Layout& layout, std::stringstream& stream) {
		if(!generated)
			stream << layout.getWhiteSpace() << const_float << result << assign << std::to_string(value) << semi_colon << std::endl;
		
		generated = true;
	}
	
	float value;
};

struct Function: public Node {
	Function(const std::string& name): Node(name + "_result"), name(name) { }
	void generate(Layout& layout, std::stringstream& stream) override {
		if(!generated) {
			arg->generate(layout, stream);
			stream << layout.getWhiteSpace() << const_float << result << assign << name << "(" << arg->result << ")" << semi_colon << std::endl;
		}
		
		generated = true;
	}
	
	const std::string name;
	Node* arg = nullptr;
};

struct Operator: public Node {
	Operator(const std::string& name, char op): Node(name), op(op) { }
	
	const char op;
	Node* lhs = nullptr;
	Node* rhs = nullptr;
};

struct Add: public Operator {
	Add(): Operator("add_result", '+') { }
	
	void generate(Layout& layout, std::stringstream& stream) override {
		lhs->generate(layout, stream);
		rhs->generate(layout, stream);
		
		if(!generated)
			stream << layout.getWhiteSpace() << const_float << result << assign << lhs->result << space << op << space << rhs->result << semi_colon << std::endl;
		
		generated = true;
	}
};

struct Sub: public Operator {
	Sub(): Operator("sub_result", '-') { }
	
	void generate(Layout& layout, std::stringstream& stream) override {
		lhs->generate(layout, stream);
		rhs->generate(layout, stream);
		
		if(!generated)
			stream << layout.getWhiteSpace() << const_float << result << assign << lhs->result << space << op << space << rhs->result << semi_colon << std::endl;
		
		generated = true;
	}
};

struct Mult: public Operator {
	Mult(): Operator("mult_result", '*') { }
	
	void generate(Layout& layout, std::stringstream& stream) override {
		lhs->generate(layout, stream);
		rhs->generate(layout, stream);
		
		if(!generated)
			stream << layout.getWhiteSpace() << const_float << result << assign << lhs->result << space << op << space << rhs->result << semi_colon << std::endl;
		
		generated = true;
	}
};

struct Div: public Operator {
	Div(): Operator("div_result", '/') { }
	
	void generate(Layout& layout, std::stringstream& stream) override {
		lhs->generate(layout, stream);
		rhs->generate(layout, stream);
		
		if(!generated)
			stream << layout.getWhiteSpace() << const_float << result << assign << lhs->result << space << op << space << rhs->result << semi_colon << std::endl;
		
		generated = true;
	}
};

struct Mod: public Operator {
	Mod(): Operator("mod_result", '%') { }
	
	void generate(Layout& layout, std::stringstream& stream) override {
		lhs->generate(layout, stream);
		rhs->generate(layout, stream);
		
		if(!generated)
			stream << layout.getWhiteSpace() << const_float << result << assign << lhs->result << space << op << space << rhs->result << semi_colon << std::endl;
		
		generated = true;
	}
};

struct Loop: public Node {
	Loop(size_t start, size_t end): Node(""), start(start), end(end) { }
	
	void generate(Layout& layout, std::stringstream& stream) override {
		if(!generated)
		{
			stream << layout.getWhiteSpace() << "for ( int i = " << std::to_string(start) << semi_colon;
			stream << layout.getWhiteSpace() << " i < " << std::to_string(end) << semi_colon;
			stream << layout.getWhiteSpace() << " i++ )" << semi_colon << std::endl;
			stream << layout.getWhiteSpace() << "{" << std::endl;
			layout.indentLevel++;
			block->generate(layout, stream);
			layout.indentLevel--;
			stream << layout.getWhiteSpace() << "}" << std::endl;
		}
		
		generated = true;
	}
	
	size_t start, end;
	Node* block;
};

struct SampleDelay: public Node {
	SampleDelay(): Node("z1_result"),
	state(Context::get().getUniqueName("delay_state")) { }
	
	void generate(Layout& layout, std::stringstream& stream) override {
		if(!generated)
		{
			input->generate(layout, stream);
			stream << layout.getWhiteSpace() << const_float << result << assign << state << semi_colon << std::endl;
			stream << layout.getWhiteSpace() << state << assign << input->result << semi_colon << std::endl;
		}
		
		generated = true;
	}
	
	std::string state;
	Node* input;
};

const std::string prologue = R"(

void process(size_t numFrames, size_t offset) {
	
	const auto max = offset + numFrames;
	for(auto i = offset; i < max; i++) {
		
)";

class Builder {
public:
	
	void build(Node* node, Layout& layout, std::stringstream& stream) {
		stream << prologue;
		layout.indentLevel = 2;
		node->generate(layout, stream);
		stream << layout.getWhiteSpace() << "output[i]" << assign << node->result << semi_colon << std::endl;
		layout.indentLevel--;
		stream << layout.getWhiteSpace() << acc_close << std::endl;
		layout.indentLevel--;
		stream << layout.getWhiteSpace() << acc_close << std::endl;
	}
};

int main() {
	
	std::stringstream stream;
//	Add add;
	Constant c1("frequency", 440.0);
//	Constant c2("offset", 10.0);
//	Function f("sin");
//	f.arg = &c2;
//	add.lhs = &c1;
//	add.rhs = &f;
//
//	Mult m;
//	m.lhs = &add;
//	m.rhs = &c1;
	
	SampleDelay delay;
	delay.input = &c1;
	
	Builder builder;
	Layout layout;
//	Context::get().createGlobals(stream);
	builder.build(&delay, layout, stream);

	std::cout << stream.str() << std::endl;
	
	return 0;
}

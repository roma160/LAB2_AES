#include "processor.h"

#include <cstring>
#include <iomanip>
#include <algorithm>

using namespace std;

std::map<processor::command::name_t, const char*> processor::command::name_map{
	{set_c, "set_c"},
	{load_ca, "load_ca"},
	{unwrap, "unwrap"},
	{dump_ca, "dump_ca"},
	{mod_c, "mod_c"},
	{mod_ca, "mod_ca"},
};

string to_binary(processor::reg_t n)
{
	stringstream s;
	while (n > 0)
	{
		s << n % 2;
		n >>= 1;
	}
	string ret = s.str();
	reverse(ret.begin(), ret.end());
	ret = string(bitmem::item_size - ret.size(), '0') + ret;
	for (int i = bitmem::item_size - 8; i > 0; i -= 8)
		ret.insert(i, 1, '.');
	return ret;
}

std::string processor::command::str() const
{
	stringstream ret;
	ret << name_map[name] << " " << to_binary(val);
	return ret.str();
}

processor::processor(vector<string> program, const size_t ram_size):
	ram(ram_size), R1(0), PC(0), TC(0), RS(0), IR()
{
	this->program = move(program);
}

bool processor::do_tick()
{
	if (PC >= program.size()) return false;

	if (TC == 0)
	{
		// Parsing command data from string
		stringstream s;
		s << program[PC];

		string str_name;
		s >> str_name;
		reg_t name = 0;
		while (
			strcmp(str_name.c_str(), command::name_map[(command::name_t)name]) != 0 &&
			name < command::name_t::COUNT
		) name++;

		IR.name = (command::name_t) name;
		s >> IR.val;
	}
	else if (TC == 1)
	{
		switch (IR.name)
		{
		case command::set_c:
			R1 = IR.val;
			break;
		case command::dump_ca:
			ram.set(IR.val, R1);
			break;
		case command::load_ca:
			R1 = ram.get(IR.val);
			break;
		case command::unwrap:
			R1 = ram.get(R1);
			break;
		case command::mod_c:
			R1 %= IR.val;
			break;
		case command::mod_ca:
			R1 %= ram.get(IR.val);
			break;
		}
	}

	return PC < program.size();
}
void processor::end_tick()
{
	if (TC == 1) PC++;
	TC++;
	TC %= 2;
}

std::string processor::get_state() const
{
	static constexpr auto delim = "------------------------------------\n";

	stringstream ss;
	ss << delim << "Current executed line: " << IR.str() << "\n";

	ss << "\nOperand registers:\n";
	ss << "R1 = " << to_binary(R1) << "\n";

	ss << "\nRAM:\n";
	ss << ram.str() << "\n" << ram.order() << "\n";

	ss << "\nState registers:\n";
	ss << "PC = " << to_binary(PC) << "\n";
	ss << "TC = " << to_binary(TC) << "\n";
	ss << "RS = " << to_binary(RS) << "\n";

	return ss.str();
}

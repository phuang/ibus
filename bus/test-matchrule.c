#include "matchrule.c"

int main()
{
	BusMatchRule *rule;

	rule = bus_match_rule_new ("type='signal',arg0='test'");

	return 0;
}

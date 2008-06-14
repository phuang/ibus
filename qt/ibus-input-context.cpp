#include "ibus-input-context.h"

IBusInputContext::IBusInputContext (QObject * parent)
	: QInputContext (parent)
{
}

QString
IBusInputContext::identifierName ()
{
	return QString ("ibus");
}

QString
IBusInputContext::language()
{
	return QString ("");
}

void
IBusInputContext::reset()
{
}

bool
IBusInputContext::isComposing() const
{
	return false;
}

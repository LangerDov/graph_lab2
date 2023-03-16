#include "string/string.h"
class BaseCoreComponent
{
public:
	virtual const UString GetName() = 0;
private:
	virtual bool Init() = 0;
	virtual bool CleanUp() = 0;
};
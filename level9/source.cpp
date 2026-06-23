#include <cstring>
#include <unistd.h>

class N
{
public:
	N(int value) : value(value) {}

	void setAnnotation(char *str)
	{
		memcpy(annotation, str, strlen(str));
	}

	virtual int operator+(N &other)
	{
		return value + other.value;
	}

	virtual int operator-(N &other)
	{
		return value - other.value;
	}

private:
	char annotation[100];
	int value;
};

int main(int argc, char **argv)
{
	N *first;
	N *second;

	if (argc <= 1)
		_exit(1);
	first = new N(5);
	second = new N(6);
	first->setAnnotation(argv[1]);
	return *second + *first;
}

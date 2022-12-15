
class Test
{
public:
	Test(const int& ref)
		: m_ref{ ref } {}

	const int& m_ref;
};

int main()
{
	int* newInt = new int();
	Test test{ *newInt };
	delete newInt;

	int a = test.m_ref;
}


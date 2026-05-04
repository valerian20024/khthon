class MyClass
{
public:
    int integer;
    bool boolean;

    MyClass() : integer(10), boolean(true) {}

    void add(int a, int b);
};

void MyClass::add(int a, int b)
{
    integer += a + b;
}

int main()
{
    MyClass *myclass = new MyClass();
    myclass->add(1, 2);
    // In principle, we must free myclass,
    // but this is not useful for the project
    // so we leave that out
    return 0;
}

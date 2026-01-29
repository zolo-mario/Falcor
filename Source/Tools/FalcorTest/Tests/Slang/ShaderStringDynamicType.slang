import GeneratedModule;

[anyValueSize(4)]
interface IDynamicType
{
    uint eval(uint x);
}

struct DynamicType : IDynamicType
{
    uint eval(uint x)
    {
        // The function f() is defined in the generated module.
        return f(x);
    }
};

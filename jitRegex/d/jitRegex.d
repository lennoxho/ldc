import core.atomic;
import core.runtime;
import dmd.frontend;

extern (C)
void init() {
    import std.algorithm : each;
    Runtime.initialize();
    initDMD;
    findImportPaths.each!addImport;
}

extern (C)
void term() {
    Runtime.terminate();
}

shared uint nextHandle = 0;

extern (C)
uint makeJitRegex(const(char)* pattern)
{
    import std.conv;
    import std.format : format;

    auto regString = pattern.to!string;

    auto m = parseModule("test.d", format!(q{
        import std.regex;

        bool hasmatch(string str)
        {
            auto reg = ctRegex!(r"%s");
            return !matchFirst(str, reg).empty;
        }
    })(regString));

    m.fullSemantic;

    uint handle = nextHandle;
    nextHandle.atomicOp!"+="(1);
    return handle;
}

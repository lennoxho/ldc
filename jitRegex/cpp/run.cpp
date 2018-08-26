#include <cstdint>
#include <iostream>
#include <string>

#include "dmd/globals.h"
#include "driver/configfile.h"
#include "driver/ldc-version.h"
#include "driver/targetmachine.h"
#include "gen/abi.h"
#include "gen/dibuilder.h"
#include "gen/irstate.h"
#include "gen/objcgen.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"

static constexpr const char* conf_filename = "/mnt/d/lenno/source/ldc/build/bin/ldc2.conf";

namespace jitRegex {
    extern "C" {
        void init();
        void term();
        std::uint32_t makeJitRegex(const char* pattern);
        void disableGC();
    }
}

struct jit_regex_manager {
    using handle = std::uint32_t;

    jit_regex_manager() { 
        jitRegex::init(); 
    }
    ~jit_regex_manager() { jitRegex::term(); }

    jit_regex_manager(const jit_regex_manager&) = delete;
    jit_regex_manager &operator=(const jit_regex_manager&) = delete;

    handle make(const std::string &pattern) const {
        return jitRegex::makeJitRegex(pattern.c_str());
    }
};

void init_passes() {
  using namespace llvm;
  // Initialize passes
  PassRegistry &registry = *PassRegistry::getPassRegistry();
  initializeCore(registry);
  initializeTransformUtils(registry);
  initializeScalarOpts(registry);
  initializeObjCARCOpts(registry);
  initializeVectorization(registry);
  initializeInstCombine(registry);
  initializeIPO(registry);
  initializeInstrumentation(registry);
  initializeAnalysis(registry);
  initializeCodeGen(registry);
  initializeGlobalISel(registry);
  initializeTarget(registry);
  initializeRewriteSymbolsLegacyPassPass(registry);
  initializeSjLjEHPreparePass(registry);
}

void init_global() {
    global._init();
    global.version = ldc::dmd_version;
    global.ldc_version = ldc::ldc_version;
    global.llvm_version = ldc::llvm_version;

    // Initialize LLVM before parsing the command line so that --version shows
    // registered targets.
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();

    init_passes();

    ConfigFile &cfg_file = ConfigFile::instance;
    // just ignore errors for now, they are still printed
    cfg_file.read(conf_filename, llvm::sys::getDefaultTargetTriple().c_str());

    auto float_abi = FloatABI::Default;
    gTargetMachine = createTargetMachine("",
                                         "",
                                         "",
                                         "",
                                         ExplicitBitness::M64,
                                         float_abi,
                                         llvm::None,
                                         llvm::CodeModel::Model::Default,
                                         llvm::CodeGenOpt::Level::Default,
                                         true);

    static llvm::DataLayout data_layout = gTargetMachine->createDataLayout();
    gDataLayout = &data_layout;

    {
        auto triple = new llvm::Triple(gTargetMachine->getTargetTriple());
        global.params.targetTriple = triple;
        global.params.isLinux = triple->isOSLinux();
        global.params.isOSX = triple->isOSDarwin();
        global.params.isWindows = triple->isOSWindows();
        global.params.isFreeBSD = triple->isOSFreeBSD();
        global.params.isOpenBSD = triple->isOSOpenBSD();
        global.params.isDragonFlyBSD = triple->isOSDragonFly();
        global.params.isSolaris = triple->isOSSolaris();
        global.params.isLP64 = true;
        global.params.is64bit = triple->isArch64Bit();
        global.params.hasObjectiveC = objc_isSupported(*triple);
        global.params.dwarfVersion = gTargetMachine->Options.MCOptions.DwarfVersion;
        global.params.mscoff = triple->isKnownWindowsMSVCEnvironment();
        if (global.params.mscoff)
          global.obj_ext = "obj";
    }

    gABI = TargetABI::getTarget();
}

int main() {
    jitRegex::disableGC();
    init_global();

    jit_regex_manager jit_regex;
    std::cout << jit_regex.make("a.*") << "\n";
}

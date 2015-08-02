PROGNAME   := xwinclone
BINDIR     := bin
INCLUDEDIR := include
SRCDIR     := src
OBJDIR     := obj
DIRMARKF   := .dir.mark
SRCSUBDIR  := . xwcArgs xwcInit xwcUtil xwcDevCtrl xwcWinUtil xwcXSrvUtil
CCFLAGS    := -Wall -MD -pipe -pedantic -Werror -std=c11 -O2 -Wshadow -flto
CCFLAGS    += -Wextra -Wstrict-prototypes -fno-asynchronous-unwind-tables
CCFLAGS    += -ffunction-sections -funroll-loops
LINKFLAGS  := -s -pipe -Wl,--gc-sections -Wl,--strip-all
LIBS       := -lX11 -lXi -lXcomposite -lXmu -lImlib2 -lm

INCLUDEFLAGS  := $(addprefix $(INCLUDEDIR)/, $(SRCSUBDIR))
RELSRCSUBDIRS := $(addprefix $(SRCDIR)/, $(SRCSUBDIR))
OBJDIRS       := $(addprefix $(OBJDIR)/, $(RELSRCSUBDIRS))
OBJECTS       := $(wildcard $(addsuffix /*.c, $(RELSRCSUBDIRS)))
OBJECTS       := $(OBJECTS:.c=.o)
OBJECTS       := $(addprefix $(OBJDIR)/, $(OBJECTS))
PROGNAME      := $(addprefix $(BINDIR)/, $(PROGNAME))
DIRMARKS      := $(addsuffix /$(DIRMARKF), $(OBJDIRS)) $(BINDIR)/$(DIRMARKF)

all : $(PROGNAME)

$(PROGNAME) : $(OBJECTS)
		gcc -o $@ $(OBJECTS) $(LINKFLAGS) $(LIBS)
	
$(OBJDIR)/%.o : %.c | $(DIRMARKS)
		gcc -o $@ -c $< $(CCFLAGS) $(addprefix -I, $(INCLUDEFLAGS))
	
$(DIRMARKS):
		-mkdir -p $(dir $@)
		-touch $@

.PHONY : clean
 
clean :
		rm -rf $(OBJDIR) $(BINDIR)
 
-include $(wildcard $(addsuffix /*.d, $(OBJDIRS)))

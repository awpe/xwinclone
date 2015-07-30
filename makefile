PROGNAME   := xwinclone
BINDIR     := bin
INCLUDEDIR := include
SRCDIR     := src
OBJDIR     := obj
SRCSUBDIR  := . xwcArgs xwcInit xwcUtil xwcDevCtrl xwcWinUtil xwcXSrvUtil
CCFLAGS    := -Wall -MD -pipe -pedantic -Werror -std=c11 -O2
LINKFLAGS  := -s -pipe
LIBS       := -lX11 -lXi -lXcomposite -lXmu -lImlib2 -lm

INCLUDEFLAGS  := $(addprefix $(INCLUDEDIR)/, $(SRCSUBDIR))
RELSRCSUBDIRS := $(addprefix $(SRCDIR)/, $(SRCSUBDIR))
OBJDIRS       := $(addprefix $(OBJDIR)/, $(RELSRCSUBDIRS))
OBJECTS       := $(wildcard $(addsuffix /*.c, $(RELSRCSUBDIRS)))
OBJECTS       := $(OBJECTS:.c=.o)
OBJECTS       := $(addprefix $(OBJDIR)/, $(OBJECTS))
PROGNAME      := $(addprefix $(BINDIR)/, $(PROGNAME))
DIRMARKS      := $(addsuffix /.f, $(OBJDIRS)) $(BINDIR)/.f

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

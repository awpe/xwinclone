PrgName = xwinclone

CC      = gcc

CFLAGS  = -Wall -pedantic -std=c11 -Og -Werror

ODIR    = obj

LIBS    = -lX11 -lXmu -lXcomposite -lImlib2 -lXi -lm

DEPS    = xwc.h xwcArgs.h xwcDevCtrl.h xwcInit.h xwcWinUtil.h xwcXSrvUtil.h

_OBJ    = $(patsubst %.h,%.o,$(DEPS)) xwinclone.o 

###############################################################################

OBJ     = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
		@echo compiling $@
		$(CC) -c -o $@ $< $(CFLAGS)

$(PrgName): $(OBJ)
		@echo $(OBJ)
		@echo linking $@
		gcc -o $@ $^ $(CFLAGS) $(LIBS)

clean:
		rm -f $(ODIR)/*.o $(PrgName)

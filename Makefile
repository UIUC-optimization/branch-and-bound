
# To use this makefile, do the following:
# 1. Update $(SRCS) to reflect the list of files you want to compile
# 2. Set $(OBJDIR) to the directory in which you want the intermediate files to be placed (and 
#    make sure the directory exists)
# 3. Change $(EXEC) to be the name you want for your executable

SRCS = bfstree.cpp brfstree.cpp btree.cpp cbfstree.cpp cdbfstree.cpp dfstree.cpp
CFLAGS = 
LDFLAGS = 

# You can leave this stuff alone for the most part; it sets the right C++ standard, tells the
# compiler to print output nicely, and prints some useful warning messages

CC = colorg++ 
STD = -std=c++0x
FORMAT = -fmessage-length=100 -fno-pretty-templates
WARNINGS = -Wempty-body -Wall -Wno-sign-compare
DEBUGFLAGS = -g -pg
OPTFLAGS = -O2
OBJDIR = obj
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.o))
DOBJS = $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.debug))

.DEFAULT_GOAL := btree
.PHONY: all btree btree_d clean debug

$(OBJDIR)/%.o : %.cpp $(OBJDIR)/%.d
	@echo; echo "Compiling $@ with $(CFLAGS) $(OPTFLAGS)"; echo "---"
	@$(CC) $(STD) $(WARNINGS) $(FORMAT) -o $@ $(CFLAGS) $(OPTFLAGS) -c $<

$(OBJDIR)/%.debug : %.cpp $(OBJDIR)/%.d
	@echo; echo "Compiling $@ with $(CFLAGS) $(DEBUGFLAGS)"; echo "---"
	@$(CC) $(STD) $(WARNINGS) $(FORMAT) -o $@ $(CFLAGS) $(DEBUGFLAGS) -c $<

# Use the -MM option for g++ or gcc to automatically determine dependency structure for $(SRCS).
# This will get stuck into a $(OBJDIR)/<sourcename>.d file, which the object files depend on.  Then,
# whenever any file in the dependency structure changes, we'll rebuild and remake.  Slick!
$(OBJDIR)/%.d : %.cpp 
	@$(SHELL) -ec "$(CC) $(STD) -MM $(CFLAGS) $< | \
		sed 's/\($*\)\.o[ :]*/$(OBJDIR)\/\1.o $(OBJDIR)\/\1.debug : /g' > $@; [ -s $@ ] || rm -f $@"

all: btree btree_d

debug: btree_d

btree: $(OBJS)
	@echo "Done."

btree_d: $(DOBJS)
	@echo "Done."

clean:
	-rm $(OBJDIR)/*.d $(OBJDIR)/*.debug $(OBJDIR)/*.o;

include $(addprefix $(OBJDIR)/,$(SRCS:.cpp=.d))

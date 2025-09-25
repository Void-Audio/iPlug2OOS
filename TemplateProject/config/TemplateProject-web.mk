# IPLUG2_ROOT should point to the top level IPLUG2 folder from the project folder
# By default, that is three directories up from /Examples/TemplateProject/config
IPLUG2_ROOT = ../../iPlug2

FORTRAN_ROOT = ../../kernel

include ../../common-web.mk

SRC += $(PROJECT_ROOT)/TemplateProject.cpp

FORTRAN_OBJS = \
    $(FORTRAN_ROOT)/VOID_Verb.o \
    $(FORTRAN_ROOT)/numtype.o \
    $(FORTRAN_ROOT)/ringBuffer.o 

FORTRAN_LIBS = -L$(FORTRAN_ROOT) -lVOID_Verb -lgfortran

CXXFLAGS += -I$(FORTRAN_ROOT)

ifeq ($(PLATFORM),native)
$(TARGET): $(OBJS) $(FORTRAN_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(FORTRAN_LIBS)
endif

WEB_CFLAGS += -DIGRAPHICS_NANOVG -DIGRAPHICS_GLES2
WAM_LDFLAGS += -O0 -s EXPORT_NAME="'ModuleFactory'" -s ASSERTIONS=0
WEB_LDFLAGS += -O0 -s ASSERTIONS=0
WEB_LDFLAGS += $(NANOVG_LDFLAGS)

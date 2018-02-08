TOPDIR := $(shell pwd)
BUILDDIR := $(TOPDIR)/build
SRC := projector.cpp serial.cpp
HDR := $(wildcard *.h)
OBJ := $(addprefix $(BUILDDIR)/,$(notdir $(SRC:.cpp=.o)))
CC=gcc
LD=gcc
CXX=g++

TEST_TARGETS := $(basename $(notdir $(wildcard $(TOPDIR)/test_*.cpp)))

GTEST_TOPDIR := $(TOPDIR)/googletest
GTEST_DIR := $(GTEST_TOPDIR)/googletest
GTEST_SRC := $(GTEST_DIR)/src/gtest-all.cc

GMOCK_DIR := $(GTEST_TOPDIR)/googlemock
GMOCK_SRC := $(GMOCK_DIR)/src/gmock-all.cc
GMOCK_LIB := $(BUILDDIR)/libgmock.a

TEST_CXXFLAGS := -isystem $(GTEST_DIR)/include -isystem $(GMOCK_DIR)/include -pthread -I$(TOPDIR) -g -std=c++11
TEST_LDFLAGS := -pthread -Wl,-rpath=$(BUILDDIR) -L$(LIBDIR) -ldotdetector $(GMOCK_LIB) $(LIBS)

#vpath %.cpp $(TOPDIR)

.PHONY: all clean test

all: test

$(BUILDDIR)/%.o: %.cpp $(HDR) Makefile | $(BUILDDIR) $(GTEST_TOPDIR)
	$(CXX) -c $(TEST_CXXFLAGS) $< -o $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)

$(GTEST_TOPDIR):
	git clone --depth 1 https://github.com/google/googletest.git $@
	cd $(GMOCK_DIR) && autoreconf -fvi
$(GTEST_SRC): | $(GTEST_TOPDIR)
$(GMOCK_SRC): | $(GTEST_TOPDIR)
$(BUILDDIR)/gtest-all.o: $(GTEST_SRC) | $(BUILDDIR)
	$(CXX) -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) \
		-isystem $(GMOCK_DIR)/include -I$(GMOCK_DIR) \
		-pthread -c $(GTEST_SRC) -o $@
$(BUILDDIR)/gmock-all.o: $(GMOCK_SRC) | $(BUILDDIR)
	$(CXX) -isystem $(GTEST_DIR)/include -I$(GTEST_DIR) \
	       -isystem $(GMOCK_DIR)/include -I$(GMOCK_DIR) \
	       -pthread -c $(GMOCK_SRC) -o $@
$(GMOCK_LIB): $(BUILDDIR)/gtest-all.o $(BUILDDIR)/gmock-all.o
	$(AR) -rv $@ $^

define MAKE_TEST
$(1)_objects := $(addprefix $(BUILDDIR)/,$(1).o)
$(1)_objects += $(OBJ)
$(BUILDDIR)/$(1): $(GMOCK_LIB) $(LIBNAME) $$($(1)_objects) | $(BUILDDIR)
	$(CXX) $$^ $(TEST_LDFLAGS) -o $$@

endef
$(foreach __t,$(TEST_TARGETS),$(eval $(call MAKE_TEST,$(__t))))

define \n


endef

test: $(addprefix $(BUILDDIR)/,$(TEST_TARGETS))
	@$(foreach __t,$(TEST_TARGETS),$(BUILDDIR)/$(__t) --gtest_color=yes$(\n))


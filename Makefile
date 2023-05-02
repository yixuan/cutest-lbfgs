FC = gfortran
FCFLAGS = -O2 -mtune=native

CXX = g++
CPPFLAGS = -DNDEBUG -I$(CUTEST)/include -I./include
CXXFLAGS = -std=c++11 -O2 -mtune=native
LDFLAGS = -L$(CUTEST)/objects/$(MYARCH)/double -lcutest -lgfortran

LBFGS_OBJ = lbfgs.o
LBFGSB_OBJ = blas.o lbfgsb.o linpack.o timer.o
SOLVER_OBJ = $(LBFGS_OBJ) $(LBFGSB_OBJ)
INTERFACE_OBJ = boxconstr_lbfgsb_interface.o boxconstr_lbfgspp_interface.o interface.o
RUN_OBJ = run_boxconstr.o

# https://stackoverflow.com/a/10172729
# https://stackoverflow.com/a/58541640
BOXCONSTR = $(shell ls problems/boxconstr)
BOXCONSTR_PATH = $(addprefix problems/boxconstr/,$(BOXCONSTR))
BOXCONSTR_TARGET = $(addsuffix /run.out,$(BOXCONSTR_PATH))
BOXCONSTR_OBJ = $(addsuffix /ELFUN.o,$(BOXCONSTR_PATH)) \
	$(addsuffix /GROUP.o,$(BOXCONSTR_PATH)) \
	$(addsuffix /RANGE.o,$(BOXCONSTR_PATH))

.PHONY: all headers echo run clean

all: headers $(SOLVER_OBJ) $(INTERFACE_OBJ) $(RUN_OBJ) $(BOXCONSTR_TARGET)
headers: include/Eigen include/LBFGSpp

####### Download Eigen and LBFGS++ #######
include/eigen-3.4.0.tar.bz2:
	@mkdir -p include
	@echo Downloading Eigen...
	cd include && wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.bz2

include/lbfgspp.zip:
	@mkdir -p include
	@echo Downloading LBFGS++...
	cd include && wget https://github.com/yixuan/LBFGSpp/archive/refs/heads/master.zip -O lbfgspp.zip

include/Eigen: include/eigen-3.4.0.tar.bz2
	if [ ! -d "include/Eigen" ]; then \
		cd include && tar -xf eigen-3.4.0.tar.bz2 && mv eigen-3.4.0/Eigen . && rm -r eigen-3.4.0; \
	fi

include/LBFGSpp: include/lbfgspp.zip
	if [ ! -d "include/LBFGSpp" ]; then \
		cd include && unzip lbfgspp.zip && mv LBFGSpp-master/include/* . && rm -r LBFGSpp-master; \
	fi
##########################################

# Compile solver files
lbfgs.o: solvers/lbfgs/lbfgs.f
	$(FC) $(FCFLAGS) -c $< -o $@
blas.o: solvers/lbfgsb/blas.f
	$(FC) $(FCFLAGS) -c $< -o $@
lbfgsb.o: solvers/lbfgsb/lbfgsb.f
	$(FC) $(FCFLAGS) -c $< -o $@
linpack.o: solvers/lbfgsb/linpack.f
	$(FC) $(FCFLAGS) -c $< -o $@
timer.o: solvers/lbfgsb/timer.f
	$(FC) $(FCFLAGS) -c $< -o $@

# Compile interface files
boxconstr_lbfgsb_interface.o: boxconstr_lbfgsb_interface.cpp interface.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
boxconstr_lbfgspp_interface.o: boxconstr_lbfgspp_interface.cpp interface.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
interface.o: interface.cpp interface.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Runners
run_boxconstr.o: run_boxconstr.cpp interface.h
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@

# Targets for box-constrained problems
$(BOXCONSTR_TARGET): %/run.out: %/ELFUN.f %/GROUP.f %/RANGE.f $(LBFGSB_OBJ) $(INTERFACE_OBJ) run_boxconstr.o
	$(FC) $(FCFLAGS) -c $*/ELFUN.f -o $*/ELFUN.o
	$(FC) $(FCFLAGS) -c $*/GROUP.f -o $*/GROUP.o
	$(FC) $(FCFLAGS) -c $*/RANGE.f -o $*/RANGE.o
	$(CXX) $(CXXFLAGS) $*/ELFUN.o $*/GROUP.o $*/RANGE.o $(LBFGSB_OBJ) $(INTERFACE_OBJ) run_boxconstr.o $(LDFLAGS) -o $@

# For debugging purposes
echo:
	@echo $(BOXCONSTR)
	@echo $(BOXCONSTR_PATH)
	@echo $(BOXCONSTR_TARGET)
	@echo $(BOXCONSTR_OBJ)

run: $(BOXCONSTR_TARGET)
	@for path in $(BOXCONSTR_PATH); do \
		cd $$path && \
		# pwd && \
		(./run.out || exit 0) && \
		echo && \
		cd ../../..; \
	done

clean:
	-rm $(SOLVER_OBJ) $(INTERFACE_OBJ) $(RUN_OBJ)
	-rm $(BOXCONSTR_OBJ)
	-rm $(BOXCONSTR_TARGET)


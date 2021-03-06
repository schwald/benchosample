
/*
 Measuring merge duration for different value sizes
 1 column, random strings. varying lengths.
 table size fix
*/

#include "bencho.h"
#include "PlotterGnuplot.h"
#include "PlotterPython.h"
#include "PlotterR.h"

#include <iostream>

class Stride: public AbstractBenchmark
{
private:
    int* p;
    int** array_seq_f;
    int** array_seq_b;
    int** array_ran; 
    int sum;
    unsigned long long jumps;
    unsigned long long size;
   
public:

    static std::string name() { return "Stride"; }


    void initialize()
    {
        setName("Stride");
        setSequenceId("stride");
        setWarmUpRuns(0);
        setMaxRuns(1);
        
        
        addPerformanceCounter("PAPI_TOT_CYC");
        // addPerformanceCounter("PAPI_L1_DCM");
        // addPerformanceCounter("PAPI_L2_DCM");
        // addPerformanceCounter("PAPI_L3_TCM");
        // addPerformanceCounter("PAPI_TLB_DM");
        

        std::unique_ptr<Parameter> stride(new Parameter("stride", 1, 262144+1, 4, ParameterType::Multiply));
        std::unique_ptr<Parameter> jumps(new Parameter("jumps", 4096));
        addParameter(std::move(stride));
        addParameter(std::move(jumps));
        
        addTestSeries(0, "random");
        addTestSeries(1, "sequential_forwards");
        addTestSeries(2, "sequential_backwards");

        setAggregatingFunction(AggregationType::Average); 

        std::unique_ptr<AbstractPlotter> plotterGnuplot(new PlotterGnuplot());
        std::unique_ptr<AbstractPlotter> plotterPython(new PlotterPython());
        std::unique_ptr<AbstractPlotter> plotterR(new PlotterR());
        addPlotter(std::move(plotterGnuplot));
        addPlotter(std::move(plotterPython));
        addPlotter(std::move(plotterR));
    }
    
    void prepareStart()
    {
		
    }
    
    void finalize()
    {
		
    }
    
    void prepareCombination(std::map<std::string, int> parameters, int /*combination*/)
    {
        int stride = parameters["stride"];
        jumps = parameters["jumps"];
        size = (size_t)parameters["stride"] * (size_t)(parameters["jumps"]+1);
        
        srand(time(NULL));
        
        if(!posix_memalign((void**)&array_seq_f, 4096, sizeof(int*) * size)) std::cerr << "Couldn't align memory." << std::endl;
        if(!posix_memalign((void**)&array_seq_b, 4096, sizeof(int*) * size)) std::cerr << "Couldn't align memory." << std::endl;
        if(!posix_memalign((void**)&array_ran, 4096, sizeof(int*) * size)) std::cerr << "Couldn't align memory." << std::endl;
        
        std::vector<size_t> v;
        v.reserve(size);
        
        for(unsigned long long i=1; i<jumps;i++)
            v.push_back(i*stride);
        
        // random
        size_t i = 0; size_t r;
        for(size_t m = jumps-1; m>1; --m)
        {
            do {
                r = rand() % m;
            } while (v[r] ==  i);
            array_ran[i] = (int*)&(array_ran[v[r]]);
            i = v[r];
            std::swap(v[r], v[m-1]);
        }
        array_ran[i] = (int*)&(array_ran[v[0]]);
        array_ran[v[0]] = NULL;
        
        // sequential
        for(unsigned long long k=0; k<jumps;k++)
        {
            array_seq_f[k*stride] = (int*)&(array_seq_f[(k+1)*stride]);
        }
        array_seq_f[jumps*stride] = NULL;
        
        // sequential
        for(unsigned long long k=jumps; k>0;k--)
        {
            array_seq_b[k*stride] = (int*)&(array_seq_b[(k-1)*stride]);
        }
        array_seq_b[0] = NULL;

    }
    
    void finishCombination(std::map<std::string, int> /*parameters*/, int /*combination*/)
    {
        delete array_seq_f;
        delete array_seq_b;
        delete array_ran;
    }
    
    void prepareRun(std::map<std::string, int> parameters, int /*combination*/, int test_series_id, int /*run*/)
    {
		clear();
		sum = 0;
        
        switch (test_series_id)
		{
            case 0:
                p = array_ran[0];
                break;
            case 1:
                p = array_seq_f[0];
                break;
            case 2:
                p = array_seq_b[parameters["stride"]*parameters["jumps"]];
                break;
        }
        
	}
    
    void finishRun(std::map<std::string, int> /*parameters*/, int /*combination*/, int /*test_series_id*/, int /*run*/)
    {
		std::cout << "sum: " << sum << p << std::endl;
	}
    
    void doTheTest(std::map<std::string, int> /*parameters*/, int /*combination*/, int /*test_series_id*/, int /*run*/)
    {
        //while (p != NULL) {
		//	p = *((int**)p);
        //}
        
        for (size_t i=0; i<jumps-1; ++i) {
            p = *((int**)p);
        }
	}
};  


MAIN(Stride)

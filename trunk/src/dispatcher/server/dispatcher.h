#ifndef __DISPATCHER_H__
#define __DISPATCHER_H__

#define DISPATCHER_VERSION 102

//BEGIN_GIGABASE_NAMESPACE

class DispatcherServer {
  private:
	char* serverURL;
//	dbEvent TerminationEvent;
    
  public:
    void run(int argc, char* argv[]);

    DispatcherServer();
    virtual~DispatcherServer();
};

//END_GIGABASE_NAMESPACE

#endif

#ifndef __SQLSERVER_H__
#define __SQLSERVER_H__

//BEGIN_GIGABASE_NAMESPACE

class SqlServer{
  private:
	char* serverURL;
    
  public:
    void run(int argc, char* argv[]);

//	dbEvent TerminationEvent;
	
    SqlServer();
    virtual~SqlServer();
};

//END_GIGABASE_NAMESPACE

#endif

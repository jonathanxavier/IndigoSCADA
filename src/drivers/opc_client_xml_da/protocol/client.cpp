
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <libxml/globals.h>
#include <libxml/uri.h>
#include <nanohttp/nanohttp-logging.h>
#include <libcsoap/soap-client.h>
#include <malloc.h>

#ifdef _WIN32 
#define xmlFree free
#endif

#define MAX_LINE_LENGTH 65535
#define MAX_PATH_LENGTH        1024
#define URN             "urn:examples"

/* command line options */
char itemlist[MAX_PATH_LENGTH] = "opcxmlda.itemlist";
int  sleep              = 1000;
int  max_name_length    = 31;
int  num_items          = 0;
long shmsize            = 65536;
int  debug              = 0;

/* shared memory header */
typedef struct
{
  char ident[4];          // must be "opc"
  int  maxItemNameLength; // maximum length of an item name
  int  maxNameLength;     // maximum length of the item value
  int  numItems;          // number of items in shared memory
  int  readErrorCount;    // 0...65536 incremented by each read error
  int  writeErrorCount;   // 0...65536 incremented by each write error
  int  spare[8];          // for future use
  char cspare[32];        // for future use
}SHM_HEADER;

void printusage(char *FileName)
{
  printf("Usage: %s [URL] [METHOD] <-itemlist=filename> <-shm=filename> <-mbx=filename> <-sleep=milliseconds> <-max_name_length=char> <-shmsize=bytes> <-debug>\n\n", FileName);

  printf("[URL] is the url of the OPC XML-DA server.\n");
  printf("[METHOD] is the method to call. [METHOD] := GetStatus | Browse | Run\n");
  printf("[URL] and [METHOD] are mandatory and must be the first 2 parameters.\n\n");

  printf("Defaults:\n");
  printf("-itemlist=opcxmlda.itemlist                                                        # may be created by Browse\n");
  printf("-sleep=1000                                                                        # time between read calls\n");
  printf("-max_name_length=31                                                                # max length of result name\n"); 
  printf("-shmsize=65536                                                                     # total size of the shared memory\n\n");

  printf("Example for creating opcxmlda.itemlist:\n");
  printf("%s http://server/opcxmlda/xmldaserver Browse > opcxmlda.itemlist\n", FileName);
}

// watchdog
static char startme[512] = "";
static int watchcnt1 = 0;
void *watchdog_thread(void *arg)
{
  int cnt1 = -1;

  printf("watchdog thread starting\n");
  while(1)
  {
    Sleep(10*sleep);
    if(cnt1 == watchcnt1) break;
    cnt1 = watchcnt1;
  }
  printf("startme again # %s\n",startme);
#ifdef unix
  rlexec(startme);
#endif
  exit(0); // pcontrol may start me again if rlexec fails
  return arg;
}

void *reader_thread(void *arg)
{
  SoapCtx *ctx, *ctx2;
  xmlNodePtr xmlcur,xmlitem,xmlitem2;
  herror_t err;
  char *itemname,*itemvalue,buf[1024], *cptr;
  int ret;
  //THREAD_PARAM *p = (THREAD_PARAM *) arg;
  //const char *url = (const char *) p->user;
  const char *url;
  //rlMailbox rlmbx(mbx);

  // read mbx until it is empty
  //rlmbx.clear();
  printf("reader_thread starting\n");

  // wait for commands from clients
  while(1)
  {
    //ret = rlmbx.read(buf,sizeof(buf)); // read "itemname,itemvalue\n"
    if(ret <= 0) continue;
    itemname = itemvalue = &buf[0];
    cptr = strchr(buf,',');
    if(cptr != NULL)
    {
      *cptr = '\0';
      cptr++;
      itemvalue = cptr;
      cptr = strchr(itemvalue,'\n');
      if(cptr != NULL) *cptr = 0;
    }
    if(debug)
    {
      printf("reader_thread Write itemname=%s itemvalue=%s\n",itemname,itemvalue);
    }

    //p->thread->lock();

    /* create a SoapCtx object */
    err = soap_ctx_new_with_method(URN, "Write", &ctx);
    if (err != H_OK)
    {
      log_error4("%s():%s [%d]", herror_func(err),
                 herror_message(err), herror_code(err));
      herror_release(err);
      goto end_of_while;
    }

    /* create the ItemList */
    xmlitem = soap_env_add_item(ctx->env, "xsd:element", "ItemList", "");

    xmlcur = ctx->env->cur;
    ctx->env->cur = xmlitem;
    xmlitem2 = soap_env_add_item(ctx->env, "xsd:string", "Items", NULL);
    if (!xmlNewProp(xmlitem2, BAD_CAST "ItemName", BAD_CAST itemname))
    {
      log_error1("Can not create new xml attribute ItemName");
      goto end_of_while;
    }
    ctx->env->cur = xmlitem2;
    if(isdigit(itemvalue[0]))
    {
      soap_env_add_item(ctx->env,"xsd:double","Value",itemvalue);
    }
    else if(strcmp(itemvalue, "true") == 0 || strcmp(itemvalue, "false") == 0)
    {
      soap_env_add_item(ctx->env, "xsd:boolean", "Value", itemvalue);
    }
    else
    {
      soap_env_add_item(ctx->env,"xsd:istring","Value",itemvalue);
    }
    ctx->env->cur = xmlitem;
    ctx->env->cur = xmlcur;

    /* invoke */
    //err = soap_client_invoke(ctx, &ctx2, url, "");
    err = soap_client_invoke(ctx, &ctx2, url, "http://opcfoundation.org/webservices/XMLDA/1.0/Write");
    if (err != H_OK)
    {
      log_error4("[%d] %s(): %s ", herror_code(err),
                 herror_func(err), herror_message(err));
      herror_release(err);
      soap_ctx_free(ctx);
      goto end_of_while;
    }

    /* print the result */
    if(debug)
    {
      printf("reader_thread result:\n");
      soap_xml_doc_print(ctx2->env->root->doc);
    }  

end_of_while:
    /* free the objects */
    soap_ctx_free(ctx2);
    soap_ctx_free(ctx);

    //p->thread->unlock();
  }
  return NULL;
}

int write_to_shared_memory(SoapCtx *ctx, void *shmadr)
{
  SHM_HEADER *shmheader;
  char       *shmnames,*itemname,*itemvalue,buf[1024];
  const char *cptr,*result_id;
  xmlNodePtr rootnode, ritemlist, items, value, node;
  int        i;

  if(ctx == NULL || shmadr == NULL) return -1;
  shmheader = (SHM_HEADER *) shmadr;
  shmnames  = (char *) shmadr;
  shmnames += sizeof(SHM_HEADER);
  rootnode = ctx->env->cur->children;

  node = rootnode;
  while(node != NULL)
  {
    if(strcmp((const char *) node->name,"RItemList") == 0) goto ritemlist_found;
    node = node->next;
  }
  printf("RItemList not found\n");
  return -1;

ritemlist_found:
  ritemlist = node;
  items = ritemlist->children;
  if(strcmp((const char *) items->name,"Items") == 0) goto items_found;
  printf("Items not found\n");
  return -1;

items_found:
  i = 0;
  node = items;
  while(node != NULL && i < shmheader->numItems)
  {
    if(strcmp((const char *) node->name, "Items") == 0)
    {
      result_id = (const char *) xmlGetProp(node, (xmlChar*) "ResultID");
      value = node->children;
      while(value != NULL)
      {
        if(value != NULL && strcmp((const char *) value->name, "Value") == 0)
        {
          cptr = (const char *) xmlNodeGetContent(value);
          if(cptr != NULL) 
          {
            if(strlen(cptr) < sizeof(buf)-1) strcpy(buf,cptr);
            else                             strcpy(buf,"too long");
            buf[shmheader->maxNameLength] = '\0';
            itemname  = shmnames + i*(shmheader->maxItemNameLength+1 + shmheader->maxNameLength+1);
            itemvalue = itemname + shmheader->maxItemNameLength + 1;
            if((result_id != NULL && strcmp(result_id,"E_UNKNOWN") == 0))
            {
              shmheader->readErrorCount++;
              if(shmheader->readErrorCount > 256*256) shmheader->readErrorCount = 0;
              strcpy(itemvalue,"ERROR");
            }
            else 
            {
              strcpy(itemvalue,buf);
            }
            if(debug) printf("write_to_shm: %s=%s\n",itemname,itemvalue);
            xmlFree((void*) cptr);
          }  
          i++;
          break;
        }
        value = value->next;
      }  
      if(result_id != NULL) xmlFree((void*) result_id);
    }
    node = node->next;
  }

  return 0;
}

int run(const char *url, int maxItemNameLength)
{
  long shmsize_needed = ((maxItemNameLength+1)+(max_name_length+1))*num_items + sizeof(SHM_HEADER);
  FILE *fin;
  char buf[1024], *cptr;
  void *shmadr;
  SHM_HEADER *shmheader;
  int  i;
  SoapCtx *ctx, *ctx2;
  xmlNodePtr xmlcur,xmlitem;
  herror_t err;

  if(url == NULL) return 1;

  // print shm parameters
  printf("maxItemNameLength=%d max_name_length=%d shmsize=%ld shmsize_needed=%ld num_items=%d\n",
          maxItemNameLength,   max_name_length,   shmsize,    shmsize_needed,    num_items);
  //if(shmsize_needed > shmsize)
  //{
  //  printf("shmsize too small -> increase it\n");
  //  return 1;
  //}
  //if(maxItemNameLength <= 0 || max_name_length <= 0 || shmsize <= 0 || shmsize_needed <= 0)
  //{
  //  printf("some values are negative or 0\n");
  //  return 1;
  //}
  if(maxItemNameLength >= (int) (sizeof(buf) - 100) || max_name_length >= (int) (sizeof(buf) - 100))
  {
    printf("name is bigger than buf length = %d\n", (int) sizeof(buf));
    return 1;
  }

  // init shared memory
  //rlSharedMemory rlshm = rlSharedMemory(shm, (unsigned long) shmsize);
  //if(rlshm.status != rlSharedMemory::OK)
  //{
  //  printf("shared memory status is not OK\n");
  //  return 1;
  //}
  //shmadr = rlshm.getUserAdr();
  //if(shmadr == NULL)
  //{
  // printf("shmadr = NULL\n");
  //  return 1;
  //}
  //memset(shmadr,0,shmsize);

  // read itemlist to shared memory
  fin = fopen(itemlist,"r");
  if(fin == NULL)
  {
    printf("could not open itemlist %s\n",itemlist);
    return 1;
  }

  printf("01\n");

  shmadr = (void *)malloc(2000);

  i = 0;
  while(fgets(buf,sizeof(buf)-1,fin) != NULL)
  {
    if(buf[0] > ' ' && buf[0] != '#')
    {
      cptr = strchr(buf,'\n');
      if(cptr != NULL) *cptr = '\0';
      cptr = (char *) shmadr;
      cptr += sizeof(SHM_HEADER) + (i*(maxItemNameLength+1 + max_name_length+1));
      strcpy(cptr,buf);
      i++;
    }
  }
  //fclose(fin);

  printf("02\n");

  // init header in shared memory
  shmheader = (SHM_HEADER *) shmadr;
  shmheader->maxItemNameLength = maxItemNameLength;
  shmheader->maxNameLength     = max_name_length;
  shmheader->numItems          = num_items;
  shmheader->readErrorCount    = 0;
  shmheader->writeErrorCount   = 0;
  strcpy(shmheader->ident,"opc");

  printf("03\n");
  
    /* create a SoapCtx object */
    err = soap_ctx_new_with_method(URN, "Read", &ctx);
    if(err != H_OK)
    {
       log_error4("%s():%s [%d]", herror_func(err),
                  herror_message(err), herror_code(err));
       herror_release(err);
       return 1;
    }

    /* create the Read ItemList */
    xmlitem = soap_env_add_item(ctx->env, "xsd:element", "ItemList", "");
    xmlcur = ctx->env->cur;
    ctx->env->cur = xmlitem;
	
    for(i=0; i<num_items; i++)
    {
      cptr = (char *) shmadr;
      cptr += sizeof(SHM_HEADER) + (i*(maxItemNameLength+1 + max_name_length+1));
      sprintf(buf,"Items ItemName=\"%s\"",cptr);
      //if(debug) printf("soap_env_add_item %s\n", buf);
	  printf("soap_env_add_item %s\n", buf);
      soap_env_add_item(ctx->env, "xsd:string", buf, NULL);
    }
	
    ctx->env->cur = xmlcur;
  
	printf("04\n");
  /*
  else
  {
    rlString rltmp;
    rlString rlenv;
    rlenv =
      "<SOAP-ENV:Envelope xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:ZSI=\"http://www.zolera.com/schemas/ZSI/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><SOAP-ENV:Header></SOAP-ENV:Header><SOAP-ENV:Body xmlns:ns1=\"http://opcfoundation.org/webservices/XMLDA/1.0/\"><ns1:Read><ns1:Options  ReturnErrorText=\"true\" ReturnItemName=\"true\" ReturnItemPath=\"true\"></ns1:Options><ns1:ItemList>";
    for(i=0; i<shmheader->numItems; i++)
    {
      cptr = (char *) shmadr;
      cptr += sizeof(SHM_HEADER) + (i*(maxItemNameLength+1 + max_name_length+1));
      rltmp = cptr;
      cptr = strchr(rltmp.text(),'#');
      if(cptr != NULL)
      {
        *cptr = '\0';
        cptr++;
        sprintf(buf,"<ns1:Items ItemName=\"%s\" ItemPath=\"%s\" MaxAge=\"500\" ></ns1:Items>",cptr,rltmp.text());
        if(debug) printf("soap_env_add_item %s\n", buf);
        rlenv += buf;
      } 
      else
      {
        sprintf(buf,"<ns1:Items ItemName=\"%s\" ItemPath=\"\" MaxAge=\"500\" ></ns1:Items>",rltmp.text());
        if(debug) printf("soap_env_add_item %s\n", buf);
        rlenv += buf;
      }  
    }
    rlenv += "</ns1:ItemList></ns1:Read></SOAP-ENV:Body></SOAP-ENV:Envelope>";

    printf("rlenv=\n%s\n", rlenv.text());
    SoapEnv *env;
    soap_env_new_from_buffer(rlenv.text(), &env);
    ctx = soap_ctx_new(env);
  }
  */

  

  // create reader thread and the watchdog
//  rlThread reader,watchdog;
//  reader.create(reader_thread,(void *) url);
//  watchdog.create(watchdog_thread,NULL);

  // poll the OPC XML-DA server forever
  while(1)
  {
    /* invoke */
    //err = soap_client_invoke(ctx, &ctx2, url, "");
    err = soap_client_invoke(ctx, &ctx2, url, "http://opcfoundation.org/webservices/XMLDA/1.0/Read");
    if(err == H_OK)
    {
      /* print the result */
      //if(debug) 
		printf("soap_client_invoke reult:\n");
      //if(debug) 
		soap_xml_doc_print(ctx2->env->root->doc);

      /* write the result to the shared memory */
      //reader.lock();
      //write_to_shared_memory(ctx2,shmadr);
      //reader.unlock();

      /* free the objects */
      soap_ctx_free(ctx2);
    }
    else
    {
      printf("ERROR soap_client_invoke url=%s Read\n", url);
      printf("%s():%s [%d]\n", herror_func(err), herror_message(err), herror_code(err));
      herror_release(err);
    }
    watchcnt1++;
    if(watchcnt1 > 256*256) watchcnt1 = 0;
    Sleep(sleep);
  }

  return 0;
}

int getMaxItemNameLength()
{
  FILE *fin;
  char line[1024];
  int  ret,len;

  fin = fopen(itemlist,"r");
  if(fin == NULL)
  {
    printf("could not open itemlist %s\n",itemlist);
    return 0;
  }

  ret = num_items = 0;
  while(fgets(line,sizeof(line)-1,fin) != NULL)
  {
    if(line[0] > ' ' && line[0] != '#')
    {
      len = strlen(line);
      if(len > ret) ret = len;
      num_items++;
    }
  }

  fclose(fin);
  printf("num_items = %d",num_items);
  return ret;
}

int browse_childs(char *url, xmlNodePtr node)
{
  SoapCtx *ctx, *ctx2;
  herror_t err;
  const char *itempath, *itemname, *isitem, *haschildren;

  char* name = (char*)alloca(100);

  while(node != NULL)
  {
    if(strcmp((const char *) node->name, "Elements") == 0)
    {
      itempath    = (const char *) xmlGetProp(node, (xmlChar*) "ItemPath");
      itemname    = (const char *) xmlGetProp(node, (xmlChar*) "ItemName");
      isitem      = (const char *) xmlGetProp(node, (xmlChar*) "IsItem");
      haschildren = (const char *) xmlGetProp(node, (xmlChar*) "HasChildren");
      if(debug) printf("DEBUG: itempath=%s itemname=%s isitem=%s haschildren=%s\n", itempath, itemname, isitem, haschildren);
      //if(haschildren != NULL && strcmp(haschildren,"true") == 0 && isitem != NULL && strcmp(isitem,"true") == 0)
      //todo: verify with http://opcxml.demo-this.com/XmlDaSampleServer/Service.asmx
      //if(haschildren != NULL && strcmp(haschildren,"true") == 0)
      if(itempath == NULL && strcmp(haschildren,"true") == 0)
      {
        if(itemname != NULL)
        {
          /* browse childs */
          printf("#S \n");
          printf("#S %s\n",itemname);

          /* create a SoapCtx object */
          err = soap_ctx_new_with_method(URN, "Browse", &ctx);
          if (err != H_OK)
          {
            log_error4("%s():%s [%d]", herror_func(err),
                      herror_message(err), herror_code(err));
            herror_release(err);
            return -1;
          }

          /* add to the request */
          soap_env_add_item(ctx->env, "xsd:string", "ItemName", itemname);

          /* invoke */
          //err = soap_client_invoke(ctx, &ctx2, url, "");
          err = soap_client_invoke(ctx, &ctx2, url, "http://opcfoundation.org/webservices/XMLDA/1.0/Browse");
          if (err != H_OK)
          {
            log_error4("[%d] %s(): %s ", herror_code(err),
                       herror_func(err), herror_message(err));
            herror_release(err);
            soap_ctx_free(ctx);
            return -1;
          }

          /* print the result */
          if(debug) soap_xml_doc_print(ctx2->env->root->doc);

          /* browse the childs */
          browse_childs(url, ctx2->env->cur->children);

          /* free the objects */
          soap_ctx_free(ctx2);
          soap_ctx_free(ctx);
        }  
      }
      //todo: verify with http://opcxml.demo-this.com/XmlDaSampleServer/Service.asmx
      //else if(haschildren != NULL && strcmp(haschildren,"true") == 0 && isitem != NULL && strcmp(isitem,"false") == 0)
      else if(haschildren != NULL && strcmp(haschildren,"true") == 0)
      {
        if(itemname != NULL)
        {
          /* browse childs */
          printf("#C \n");
          printf("#C %s\\%s\n",itempath,itemname);
          //if(strlen(itempath) == 0) name.printf("Complex Data/%s",itemname);
          //else                      name.printf("Complex Data/%s/%s",itempath,itemname);

          /* create a SoapCtx object */
          char* rltext = (char*)alloca(513);
          sprintf(rltext,"<SOAP-ENV:Envelope xmlns:SOAP-ENC=\"http://schemas.xmlsoap.org/soap/encoding/\" xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" xmlns:ZSI=\"http://www.zolera.com/schemas/ZSI/\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"><SOAP-ENV:Header></SOAP-ENV:Header><SOAP-ENV:Body xmlns:ns1=\"http://opcfoundation.org/webservices/XMLDA/1.0/\"><ns1:Browse ItemName=\"%s\" ItemPath=\"%s\" ReturnErrorText=\"true\"></ns1:Browse></SOAP-ENV:Body></SOAP-ENV:Envelope>", itemname, itempath);
          SoapEnv *env;
          soap_env_new_from_buffer(rltext, &env);
          ctx = soap_ctx_new(env);

          /* invoke */
          err = soap_client_invoke(ctx, &ctx2, url, "http://opcfoundation.org/webservices/XMLDA/1.0/Browse");
          if (err != H_OK)
          {
            log_error4("[%d] %s(): %s ", herror_code(err),
                       herror_func(err), herror_message(err));
            herror_release(err);
            soap_ctx_free(ctx);
            return -1;
          }
          
          /* print the result */
          if(debug) soap_xml_doc_print(ctx2->env->root->doc);

          /* browse the childs */
          browse_childs(url, ctx2->env->cur->children);

          /* free the objects */
          soap_ctx_free(ctx2);
          soap_ctx_free(ctx);
        }  
      }    
      else if(haschildren != NULL && strcmp(haschildren,"false") == 0 && isitem != NULL && strcmp(isitem,"true") == 0)
      {
        if(itempath == NULL) printf("%s\n", itemname);
        else                 printf("%s#%s\n", itempath, itemname);
      }
    }  
    node = node->next;
  }
  return 0;
}

int main(int argc, char *argv[])
{
  SoapCtx *ctx, *ctx2;
  herror_t err;
  char *url,*method,*arg;
  int i;

  /* check the command line parameters */
  if(argc < 3)
  {
    printusage(argv[0]);
    return 1;
  }
  arg = argv[0];
  strcpy(startme,arg);
  for(i=1; i<argc; i++)
  {
    arg = argv[i];
    strcat(startme," ");
    strcat(startme,arg);
  }
  url    = argv[1];
  method = argv[2];
  for(i=3; i<argc; i++)
  {
    arg = argv[i];
    if(strlen(arg) <= MAX_PATH_LENGTH)
    {
      if(strncmp(arg,"-itemlist=",10) == 0)
      {
        sscanf(arg,"-itemlist=%s",itemlist);
      }
      if(strncmp(arg,"-sleep=",7) == 0)
      {
        sscanf(arg,"-sleep=%d",&sleep);
        if(sleep < 10) sleep = 10;
      }
      if(strncmp(arg,"-max_name_length=",17) == 0)
      {
        sscanf(arg,"-max_name_length=%d",&max_name_length);
        if(max_name_length < 31) max_name_length = 31;
      }
      if(strncmp(arg,"-shmsize=",8) == 0)
      {
        sscanf(arg,"-shmsize=%ld",&shmsize);
        if(shmsize < 1) shmsize = 1;
      }
      if(strcmp(arg,"-debug") == 0)
      {
        debug = 1;
      }
    }
    else
    {
      printf("%s is too long.\n", arg);
      return 1;
    }
  }

  /* init cSOAP client */
  err = soap_client_init_args(argc, argv);
  if (err != H_OK)
  {
    log_error4("%s():%s [%d]", herror_func(err),
               herror_message(err), herror_code(err));
    herror_release(err);
    return 1;
  }

  if(strcmp(method,"GetStatus") == 0)
  {
    /* create a SoapCtx object */
    printf("soap_ctx_new_with_method(%s,\"GetStatus\")\n", URN);
    err = soap_ctx_new_with_method(URN, "GetStatus", &ctx);
    if (err != H_OK)
    {
      printf("err != H_OK\n");
      log_error4("%s():%s [%d]", herror_func(err),
                 herror_message(err), herror_code(err));
      herror_release(err);
      return 1;
    }

    /* invoke */
    printf("soap_client_invoke(\"%s\")\n", url);
    err = soap_client_invoke(ctx, &ctx2, url, "http://opcfoundation.org/webservices/XMLDA/1.0/GetStatus");
    if (err != H_OK)
    {
      printf("err != H_OK\n");
      log_error4("[%d] %s(): %s ", herror_code(err),
                 herror_func(err), herror_message(err));
      herror_release(err);
      soap_ctx_free(ctx);
      return 1;
    }

    /* print the result */
    soap_xml_doc_print(ctx2->env->root->doc);

    /* free the objects */
    printf("soap_ctx_free()\n");
    soap_ctx_free(ctx2);
    soap_ctx_free(ctx);

    /* destroy the cSOAP client */
    printf("soap_client_destroy()\n");
    soap_client_destroy();

    return 0;
  }
  else if(strcmp(method,"Browse") == 0)
  {
    printf("#%s %s %s\n",argv[0],url,method);
    printf("#\n");

    /* create a SoapCtx object */
    err = soap_ctx_new_with_method(URN, "Browse", &ctx);
    if (err != H_OK)
    {
      log_error4("%s():%s [%d]", herror_func(err),
                 herror_message(err), herror_code(err));
      herror_release(err);
      return 1;
    }

    /* invoke */
    //err = soap_client_invoke(ctx, &ctx2, url, "");
    err = soap_client_invoke(ctx, &ctx2, url, "http://opcfoundation.org/webservices/XMLDA/1.0/Browse");
    if (err != H_OK)
    {
      log_error4("[%d] %s(): %s ", herror_code(err),
                 herror_func(err), herror_message(err));
      herror_release(err);
      soap_ctx_free(ctx);
      return 1;
    }

    /* print the result */
    if(debug) soap_xml_doc_print(ctx2->env->root->doc);

    /* browse the childs */
    browse_childs(url, ctx2->env->cur->children);

    /* free the objects */
    soap_ctx_free(ctx2);
    soap_ctx_free(ctx);

    /* destroy the cSOAP client */
    soap_client_destroy();

    return 0;
  }
  else if(strcmp(method,"Run") == 0)
  {
    i = getMaxItemNameLength();
    if(i > 0) i = run(url,i);
    soap_client_destroy();
    return i;
  }
  else
  {
    printusage(argv[0]);
    soap_client_destroy();
  }  

  return 0;
}

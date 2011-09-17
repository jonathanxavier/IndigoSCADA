/*
 *                         IndigoSCADA
 *
 *   This software and documentation are Copyright 2002 to 2011 Enscada 
 *   Limited and its licensees. All rights reserved. See file:
 *
 *                     $HOME/LICENSE 
 *
 *   for full copyright notice and license terms. 
 *
 */

#ifndef PROTOCOL_BASE_H
#define PROTOCOL_BASE_H

class CProtocol_Base
{
public:
    CProtocol_Base();
	virtual bool CheckCRC(CByteArray *m_RecvBufArray);
protected:
private:
};



#endif
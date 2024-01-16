# Microsoft Developer Studio Project File - Name="iec61850lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=iec61850lib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "iec61850lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "iec61850lib.mak" CFG="iec61850lib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "iec61850lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "iec61850lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "iec61850lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\src\iec61850\inc_private" /I ".\src\iec61850\inc" /I ".\src\logging" /I ".\src\mms\inc_private" /I ".\src\mms\inc" /I ".\hal\inc" /I ".\config" /I ".\src\hal\thread" /I ".\src\hal\socket" /I ".\inc" /I ".\src\common" /I ".\src\common\inc" /I ".\src\model" /I ".\src\mms\iso_mms\server" /I ".\src\hal" /I ".\src\mms\iso_acse\asn1c" /I ".\src\mms\iso_presentation\asn1c" /I ".\src\mms\iso_mms\common" /I ".\src\mms\asn1" /I ".\src\mms\iso_acse" /I ".\src\mms\iso_cotp" /I ".\src\mms\iso_session" /I ".\src\mms\iso_presentation" /I ".\src\mms\iso_client" /I ".\src\mms\iso_mms\asn1c" /I ".\src\mms\iso_server" /I ".\src\mms\iso_mms\client" /I ".\src\mms_mapping" /I ".\src\api" /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "iec61850lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I ".\src\iec61850\inc_private" /I ".\src\iec61850\inc" /I ".\src\logging" /I ".\src\mms\inc_private" /I ".\src\mms\inc" /I ".\hal\inc" /I ".\config" /I ".\src\hal\thread" /I ".\src\hal\socket" /I ".\inc" /I ".\src\common" /I ".\src\common\inc" /I ".\src\model" /I ".\src\mms\iso_mms\server" /I ".\src\hal" /I ".\src\mms\iso_acse\asn1c" /I ".\src\mms\iso_presentation\asn1c" /I ".\src\mms\iso_mms\common" /I ".\src\mms\asn1" /I ".\src\mms\iso_acse" /I ".\src\mms\iso_cotp" /I ".\src\mms\iso_session" /I ".\src\mms\iso_presentation" /I ".\src\mms\iso_client" /I ".\src\mms\iso_mms\asn1c" /I ".\src\mms\iso_server" /I ".\src\mms\iso_mms\client" /I ".\src\mms_mapping" /I ".\src\api" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "iec61850lib - Win32 Release"
# Name "iec61850lib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\AccessResult.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_acse\acse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Address.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\AlternateAccess.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\AlternateAccessSelection.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\asn1\asn1_ber_primitive_value.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\asn_codecs_prim.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\asn_SEQUENCE_OF.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\asn_SET_OF.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\asn1\ber_decode.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ber_decoder.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\asn1\ber_encoder.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\asn1\ber_integer.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ber_tlv_length.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ber_tlv_tag.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\BIT_STRING.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\BOOLEAN.c
# End Source File
# Begin Source File

SOURCE=.\src\common\buffer_chain.c
# End Source File
# Begin Source File

SOURCE=.\src\common\byte_buffer.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\impl\client_connection.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\client\client_control.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\client\client_report.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\client\client_report_control.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ConcludeRequestPDU.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ConcludeResponsePDU.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ConfirmedErrorPDU.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ConfirmedRequestPdu.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ConfirmedResponsePdu.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ConfirmedServiceRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ConfirmedServiceResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\constr_CHOICE.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\constr_SEQUENCE.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\constr_SEQUENCE_OF.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\constr_SET_OF.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\constr_TYPE.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\constraints.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\mms_mapping\control.c
# End Source File
# Begin Source File

SOURCE=.\src\common\conversions.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_cotp\cotp.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Data.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\DataAccessError.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\DataSequence.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\DefineNamedVariableListRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\DefineNamedVariableListResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\DeleteNamedVariableListRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\DeleteNamedVariableListResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\der_encoder.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\model\dynamic_model.c
# End Source File
# Begin Source File

SOURCE=.\hal\filesystem\win32\file_provider_win32.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\FloatingPoint.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\GeneralizedTime.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\GetNamedVariableListAttributesRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\GetNamedVariableListAttributesResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\GetNameListRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\GetNameListResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\GetVariableAccessAttributesRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\GetVariableAccessAttributesResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Identifier.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\common\iec61850_common.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\client\ied_connection.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\impl\ied_server.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\impl\ied_server_config.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\IndexRangeSeq.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\InformationReport.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\InitiateErrorPdu.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\InitiateRequestPdu.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\InitiateResponsePdu.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\InitRequestDetail.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\InitResponseDetail.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\INTEGER.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Integer16.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Integer32.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Integer8.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_client\iso_client_connection.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_server\iso_connection.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_common\iso_connection_parameters.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_presentation\iso_presentation.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_server\iso_server.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_session\iso_session.c
# End Source File
# Begin Source File

SOURCE=.\hal\memory\lib_memory.c
# End Source File
# Begin Source File

SOURCE=.\src\common\linked_list.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ListOfVariableSeq.c
# End Source File
# Begin Source File

SOURCE=.\src\logging\log_storage.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\mms_mapping\logging.c
# End Source File
# Begin Source File

SOURCE=.\src\common\map.c
# End Source File
# Begin Source File

SOURCE=.\src\common\mem_alloc_linked_list.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_access_result.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_association_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_common.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_connection.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_files.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_get_namelist.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_get_var_access.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_identify.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_initiate.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_journals.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_named_variable_list.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_read.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_status.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\client\mms_client_write.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\common\mms_common_msg.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_device.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_domain.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_file_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_get_namelist_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_get_var_access_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_identify_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_information_report.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_journal.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_journal_service.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\mms_mapping\mms_mapping.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_named_variable_list.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_named_variable_list_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_read_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_server.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_server_common.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_server_connection.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_status_service.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\mms_mapping\mms_sv.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\common\mms_type_spec.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\common\mms_value.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_value_cache.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\server\mms_write_service.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\MmsPdu.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\MMSString.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\model\model.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\NativeEnumerated.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\NativeInteger.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\NULL.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ObjectClass.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ObjectName.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\OCTET_STRING.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ParameterSupportOptions.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\per_decoder.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\per_encoder.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\per_support.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ReadRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ReadResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\RejectPDU.c
# End Source File
# Begin Source File

SOURCE=.\src\iec61850\server\mms_mapping\reporting.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ScatteredAccessDescription.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ServiceError.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\ServiceSupportOptions.c
# End Source File
# Begin Source File

SOURCE=.\src\common\simple_allocator.c
# End Source File
# Begin Source File

SOURCE=.\hal\socket\win32\socket_win32.c
# End Source File
# Begin Source File

SOURCE=.\src\common\string_map.c
# End Source File
# Begin Source File

SOURCE=.\src\common\string_utilities.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\StructComponent.c
# End Source File
# Begin Source File

SOURCE=.\hal\thread\win32\thread_win32.c
# End Source File
# Begin Source File

SOURCE=.\hal\time\win32\time.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\TimeOfDay.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\TypeSpecification.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\UnconfirmedPDU.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\UnconfirmedService.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Unsigned16.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Unsigned32.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\Unsigned8.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\UtcTime.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\UTF8String.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\VariableAccessSpecification.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\VariableSpecification.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\VisibleString.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\WriteRequest.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\WriteResponse.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\xer_decoder.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\xer_encoder.c
# End Source File
# Begin Source File

SOURCE=.\src\mms\iso_mms\asn1c\xer_support.c
# End Source File
# End Group
# End Target
# End Project

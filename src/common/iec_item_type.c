/*
 * This file was generated by orte-idl - DO NOT EDIT!
 */

#include "iec_item_type.h"

/****************************************************************/
/* struct - iec_item_type                                       */
/****************************************************************/

void iec_item_type_serialize(CDR_Codec *cdrCodec,iec_item_type *object) {
  CORBA_octet_serialize(cdrCodec,&(object->iec_type));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte2));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte3));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte4));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte5));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte6));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte7));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte8));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte9));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte10));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte11));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte12));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte13));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte14));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte15));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte16));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte17));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte18));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte19));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte20));
  CORBA_octet_serialize(cdrCodec,&(object->iec_obj_byte21));
  CORBA_octet_serialize(cdrCodec,&(object->cause));
  CORBA_unsigned_long_serialize(cdrCodec,&(object->msg_id));
  CORBA_long_serialize(cdrCodec,&(object->ioa_control_center));
  CORBA_unsigned_short_serialize(cdrCodec,&(object->casdu));
  CORBA_octet_serialize(cdrCodec,&(object->is_neg));
  CORBA_octet_serialize(cdrCodec,&(object->checksum));
}

void
iec_item_type_deserialize(CDR_Codec *cdrCodec,iec_item_type *object) {
  CORBA_octet_deserialize(cdrCodec,&(object->iec_type));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte2));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte3));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte4));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte5));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte6));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte7));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte8));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte9));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte10));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte11));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte12));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte13));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte14));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte15));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte16));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte17));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte18));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte19));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte20));
  CORBA_octet_deserialize(cdrCodec,&(object->iec_obj_byte21));
  CORBA_octet_deserialize(cdrCodec,&(object->cause));
  CORBA_unsigned_long_deserialize(cdrCodec,&(object->msg_id));
  CORBA_long_deserialize(cdrCodec,&(object->ioa_control_center));
  CORBA_unsigned_short_deserialize(cdrCodec,&(object->casdu));
  CORBA_octet_deserialize(cdrCodec,&(object->is_neg));
  CORBA_octet_deserialize(cdrCodec,&(object->checksum));
}

int
iec_item_type_get_max_size(ORTEGetMaxSizeParam *gms, int i) {
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_unsigned_long_get_max_size(gms, i);
  CORBA_long_get_max_size(gms, i);
  CORBA_unsigned_short_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  CORBA_octet_get_max_size(gms, i);
  return gms->csize;
}

Boolean
iec_item_type_type_register(ORTEDomain *d) {
  Boolean ret;

  ret=ORTETypeRegisterAdd(d,
                          "iec_item_type",
                          (ORTETypeSerialize)iec_item_type_serialize,
                          (ORTETypeDeserialize)iec_item_type_deserialize,
                          iec_item_type_get_max_size,
                          0);
  return ret;
}

/********************************************************************************
 * Copyright (c) 2014-2019 Cirrus Link Solutions and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0.
 *
 * SPDX-License-Identifier: EPL-2.0
 *
 * Contributors:
 *   Cirrus Link Solutions - initial implementation
 ********************************************************************************/
//apa modified, for compiling in ANSI C
#include <stdio.h>
#include <stdlib.h>
#include "pb_decode.h"
#include "pb_encode.h"
#include "tahu.h"
#include "tahu.pb.h"
#include "malloc.h" //apa+++ for alloca()

// Global sequence variable
uint64_t seq;

/*
 * Private function to decode a Metric from a stream
 */
bool decode_metric(org_eclipse_tahu_protobuf_Payload_Metric *metric, pb_istream_t *stream) {
	bool status;
	pb_istream_t substream;

	pb_wire_type_t metric_wire_type;
	uint32_t metric_tag;
	bool metric_eof;
	const pb_field_t *metric_field;
	uint64_t udest;
	int64_t dest;
	uint32_t dest32;
	float destination_float;
	uint64_t dest64;
	double destination_double;
	pb_byte_t string_size[1];
	pb_byte_t *dest_pb_byte_t;

	if (!pb_make_string_substream(stream, &substream)) {
		fprintf(stderr, "Failed to create the substream for metric decoding\n");
		return false;
	}

	while (pb_decode_tag(&substream, &metric_wire_type, &metric_tag, &metric_eof)) {
		DEBUG_PRINT(("\teof: %s\n", metric_eof ? "true" : "false"));
		DEBUG_PRINT(("\t\tBytes Remaining: %d\n", substream.bytes_left));
		DEBUG_PRINT(("\t\tWiretype: %d\n", metric_wire_type));
		DEBUG_PRINT(("\t\tTag: %d\n", metric_tag));

		if (metric_wire_type == PB_WT_VARINT) {
			DEBUG_PRINT(("\t\tMetric Wire type is PB_WT_VARINT\n"));
			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && (((metric_field->type & PB_LTYPE_VARINT) == PB_LTYPE_VARINT) ||
													((metric_field->type & PB_LTYPE_UVARINT) == PB_LTYPE_UVARINT))) {
					DEBUG_PRINT(("\t\tWire type is PB_WT_VARINT\n"));
					
					status = pb_decode_varint(&substream, &udest);
					if (status) {
						DEBUG_PRINT(("\t\tVARINT - Success - new value: %ld\n", udest));
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_alias_tag) {
							metric->has_alias = true;
							metric->alias = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_timestamp_tag) {
							metric->has_timestamp = true;
							metric->timestamp = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_datatype_tag) {
							metric->has_datatype = true;
							metric->datatype = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_is_historical_tag) {
							metric->has_is_historical = true;
							metric->is_historical = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_is_transient_tag) {
							metric->has_is_transient = true;
							metric->is_transient = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_is_null_tag) {
							metric->has_is_null = true;
							metric->is_null = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
							metric->value.int_value = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
							metric->value.long_value = udest;
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag;
							metric->value.boolean_value = udest;
						}
					} else {
						fprintf(stderr, "\t\tVARINT - Failed to decode variant!\n");
						return false;
					}
				} else if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_SVARINT) == PB_LTYPE_SVARINT)) {
					DEBUG_PRINT(("\t\tWire type is PB_WT_SVARINT\n"));
					
					status = pb_decode_svarint(&substream, &dest);
					if (status) {
						DEBUG_PRINT(("\t\tVARINT - Success - new value: %ld\n", dest));
					} else {
						fprintf(stderr, "\t\tVARINT - Failed to decode variant!\n");
						return false;
					}
				}
			}
		} else if (metric_wire_type == PB_WT_32BIT) {
			DEBUG_PRINT(("\t\tMetric Wire type is PB_WT_32BIT\n"));
			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && (((metric_field->type & PB_LTYPE_FIXED32) == PB_LTYPE_FIXED32))) {
					DEBUG_PRINT(("\t\tWire type is PB_WT_32BIT\n"));
					
					status = pb_decode_fixed32(&substream, &dest32);
					if (status) {
						DEBUG_PRINT(("\t\t32BIT - Success - new value: %d\n", dest32));
						destination_float = *((float*)&dest32);
						DEBUG_PRINT(("\t\tFLoat - Success - new value: %f\n", destination_float));
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_float_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_float_value_tag;
							metric->value.float_value = destination_float;
						}
					}
				}
			}
		} else if (metric_wire_type == PB_WT_64BIT) {
			DEBUG_PRINT(("\t\tMetric Wire type is PB_WT_64BIT\n"));
			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && (((metric_field->type & PB_LTYPE_FIXED64) == PB_LTYPE_FIXED64))) {
					DEBUG_PRINT(("\t\tWire type is PB_WT_64BIT\n"));
					
					status = pb_decode_fixed64(&substream, &dest64);
					if (status) {
						DEBUG_PRINT(("\t\t64BIT - Success - new value: %ld\n", dest64));
						destination_double = *((double*)&dest64);
						DEBUG_PRINT(("\t\tDouble - Success - new value: %f\n", destination_double));
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_double_value_tag) {
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_double_value_tag;
							metric->value.double_value = destination_double;
						}
					}
				}
			}

		} else if (metric_wire_type == PB_WT_STRING) {
			DEBUG_PRINT(("\t\tMetric Wire type is PB_WT_STRING\n"));

			for (metric_field = org_eclipse_tahu_protobuf_Payload_Metric_fields; metric_field->tag != 0; metric_field++) {
				if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_SUBMESSAGE) == PB_LTYPE_SUBMESSAGE)) {
					DEBUG_PRINT(("\t\tFound a PB_LTYPE_SUBMESSAGE\n"));
				} else if (metric_field->tag == metric_tag &&
							((metric_field->type & PB_LTYPE_FIXED_LENGTH_BYTES) == PB_LTYPE_FIXED_LENGTH_BYTES)) {
					DEBUG_PRINT(("\t\tFound a PB_LTYPE_FIXED_LENGTH_BYTES\n"));
				} else if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_STRING) == PB_LTYPE_STRING)) {
					DEBUG_PRINT(("\t\tFound a PB_LTYPE_STRING\n"));

					// Get the string size
					
					status = pb_read(&substream, string_size, 1);
					if (status) {
						DEBUG_PRINT(("\t\tString Size: %d\n", string_size[0]));
					} else {
						fprintf(stderr, "\t\tFailed to get the string size while decoding\n");
						return false;
					}

					
					//pb_byte_t dest[string_size[0]+1];
					dest_pb_byte_t = (pb_byte_t *)alloca(string_size[0]+1);
					status = pb_read(&substream, dest_pb_byte_t, string_size[0]);
					if (status) {
						dest_pb_byte_t[string_size[0]] = '\0';

						// This is either the metric name or string value
						if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_name_tag) {
							DEBUG_PRINT(("\t\tRead the Metric name! %s\n", dest_pb_byte_t));
							metric->name = (char *)malloc((strlen(dest_pb_byte_t)+1)*sizeof(char));
							strcpy(metric->name, dest_pb_byte_t);
						} else if (metric_field->tag == org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag) {
							DEBUG_PRINT(("\t\tRead the Metric string_value! %s\n", dest_pb_byte_t));
							metric->which_value = org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag;
							// JPL 04/05/17... I hope this gets FREE(string_value)'d somewhere
							metric->value.string_value =(char *)malloc((strlen(dest_pb_byte_t)+1)*sizeof(char));
							strcpy(metric->value.string_value, dest_pb_byte_t );
							// JPL 04/05/17... local memory?
							//	metric->value.string_value = dest_pb_byte_t;
						}
					} else {
						fprintf(stderr, "\t\tFailed to read the string...\n");
						return false;
					}
					
				} else if (metric_field->tag == metric_tag && ((metric_field->type & PB_LTYPE_BYTES) == PB_LTYPE_BYTES)) {
					DEBUG_PRINT(("\t\tFound a PB_LTYPE_BYTES\n"));
				//} else {
					//DEBUG_PRINT(("\t\tother: %d\n", metric_field->type);
				}
			}

		} else if (metric_wire_type == PB_WT_32BIT) {
			DEBUG_PRINT(("\t\tMetric Wire type is PB_WT_32BIT\n"));
		//} else {
			//DEBUG_PRINT(("\t\tMetric Other? %d\n", metric_wire_type);
		}
	}

	// Close the substream
	pb_close_string_substream(stream, &substream);
}

/*
 * Private function to increase the size of an array of strings
 */
int grow_char_array(char **array, int current_size, int num_new_elems) {
        const int total_size = current_size + num_new_elems;
	char *temp = (char *)realloc(*array, (total_size * sizeof(char*)));

	if (temp == NULL) {
		fprintf(stderr, "Cannot allocate more memory.\n");
		return 0;
	} else {
		*array = temp;
	}

	return total_size;
}

/*
 * Private function to increase the size of an array of Metrics
 */
int grow_metrics_array(org_eclipse_tahu_protobuf_Payload_Metric **metric_array, int current_size, int num_new_elems) {
        const int total_size = current_size + num_new_elems;
        org_eclipse_tahu_protobuf_Payload_Metric *temp = (org_eclipse_tahu_protobuf_Payload_Metric*)realloc(*metric_array,
                                                                 	(total_size * sizeof(org_eclipse_tahu_protobuf_Payload_Metric)));

	if (temp == NULL) {
		fprintf(stderr, "Cannot allocate more memory.\n");
		return 0;
	} else {
		*metric_array = temp;
	}

        return total_size;
}

/*
 * Private function to increase the size of an array of PropertyValues
 */
int grow_propertyvalues_array(org_eclipse_tahu_protobuf_Payload_PropertyValue **values_array, int current_size, int num_new_elems) {
        const int total_size = current_size + num_new_elems;
        org_eclipse_tahu_protobuf_Payload_PropertyValue *temp = (org_eclipse_tahu_protobuf_Payload_PropertyValue*)realloc(*values_array,
                                                                 (total_size * sizeof(org_eclipse_tahu_protobuf_Payload_PropertyValue)));

	if (temp == NULL) {
		fprintf(stderr, "Cannot allocate more memory.\n");
		return 0;
	} else {
		*values_array = temp;
	}

        return total_size;
}

/*
 * Add Metadata to an existing Metric
 */
void add_metadata_to_metric(org_eclipse_tahu_protobuf_Payload_Metric *metric, org_eclipse_tahu_protobuf_Payload_MetaData *metadata) {
	metric->has_metadata = true;
	metric->metadata = *metadata;
}

/*
 * Add a complete Metric to an existing Payload
 */
void add_metric_to_payload(org_eclipse_tahu_protobuf_Payload *payload, org_eclipse_tahu_protobuf_Payload_Metric *metric) {

	int size = payload->metrics_count;
	if (size == 0) {
		payload->metrics = (org_eclipse_tahu_protobuf_Payload_Metric *) calloc(1, sizeof(org_eclipse_tahu_protobuf_Payload_Metric));
		if(payload->metrics == NULL) {
			fprintf(stderr, "Cannot allocate initial memory for data\n");
		} else {
			size = 1;
		}
	} else {
		size = grow_metrics_array(&payload->metrics, size, 1);
	}

	// Assign the metric
	payload->metrics[payload->metrics_count] = *metric;

	// Increment the metric count
	payload->metrics_count++;
}

/*
 * Add a simple Property to an existing PropertySet
 */
bool add_property_to_set(org_eclipse_tahu_protobuf_Payload_PropertySet *propertyset, const char *key, uint32_t datatype, bool is_null, const void *value, size_t size_of_value) {

	int size;

	if(propertyset->keys_count != propertyset->values_count) {
		fprintf(stderr, "Invalid PropertySet!\n");
		return false;
	}

	size = propertyset->keys_count;
	if (size == 0) {
		propertyset->keys = (char **) calloc(1, sizeof(char*));
		propertyset->values = (org_eclipse_tahu_protobuf_Payload_PropertyValue *) calloc(1, sizeof(org_eclipse_tahu_protobuf_Payload_PropertyValue));
		if(propertyset->values == NULL) {
			fprintf(stderr, "Cannot allocate initial memory for data\n");
		} else {
			size = 1;
		}
	} else {
		grow_char_array(propertyset->keys, size, 1);
		size = grow_propertyvalues_array(&propertyset->values, size, 1);
	}

	// Set the key name in the array of keys
	propertyset->keys[size-1] = (char *)malloc((strlen(key)+1)*sizeof(char));
	strcpy(propertyset->keys[size-1], key);

	// Set the value components
	propertyset->values[size-1].has_type = true;
	propertyset->values[size-1].type = datatype;
	propertyset->values[size-1].has_is_null = is_null;
	if (is_null) {
		propertyset->values[size-1].is_null = true;
	}
	if (datatype == PROPERTY_DATA_TYPE_UNKNOWN) {
		fprintf(stderr, "Can't create property value with unknown datatype!\n");
	} else if (datatype == PROPERTY_DATA_TYPE_INT8) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int8_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_int_value_tag;
		propertyset->values[size-1].value.int_value = *((int8_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_INT16) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int16_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_int_value_tag;
		propertyset->values[size-1].value.int_value = *((int16_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_INT32) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int32_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_int_value_tag;
		propertyset->values[size-1].value.int_value = *((int32_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_INT64) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int64_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_long_value_tag;
		propertyset->values[size-1].value.long_value = *((int64_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_UINT8) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %u\n", datatype, *((uint8_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_int_value_tag;
		propertyset->values[size-1].value.int_value = *((uint8_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_UINT16) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %u\n", datatype, *((uint16_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_int_value_tag;
		propertyset->values[size-1].value.int_value = *((uint16_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_UINT32) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %u\n", datatype, *((uint32_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_long_value_tag;
		propertyset->values[size-1].value.long_value = *((uint32_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_UINT64) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((uint64_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_long_value_tag;
		propertyset->values[size-1].value.long_value = *((uint64_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_DATETIME) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((uint64_t *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_long_value_tag;
		propertyset->values[size-1].value.long_value = *((uint64_t *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_FLOAT) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %f\n", datatype, *((float *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_float_value_tag;
		propertyset->values[size-1].value.float_value = *((float *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_DOUBLE) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %f\n", datatype, *((double *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_double_value_tag;
		propertyset->values[size-1].value.double_value = *((double *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_BOOLEAN) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((bool *)value)));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_boolean_value_tag;
		propertyset->values[size-1].value.boolean_value = *((bool *)value);
	} else if (datatype == PROPERTY_DATA_TYPE_STRING || datatype == PROPERTY_DATA_TYPE_TEXT) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %s\n", datatype, (char *)value));
		propertyset->values[size-1].which_value = org_eclipse_tahu_protobuf_Payload_PropertyValue_string_value_tag;
		propertyset->values[size-1].value.string_value = (char *)malloc(size_of_value*sizeof(char));
		strcpy(propertyset->values[size-1].value.string_value, (char *)value);
	} else {
		fprintf(stderr, "Unknown datatype %u\n", datatype);
	}

	propertyset->keys_count++;
	propertyset->values_count++;
	DEBUG_PRINT(("Size of values in PropertySet %d\n", propertyset->keys_count));
}

/*
 * Add a PropertySet to an existing Metric
 */
void add_propertyset_to_metric(org_eclipse_tahu_protobuf_Payload_Metric *metric, org_eclipse_tahu_protobuf_Payload_PropertySet *properties) {
	metric->has_properties = true;
	metric->properties = *properties;
}

/*
 * Add a simple Metric to an existing Payload
 */
void add_simple_metric(org_eclipse_tahu_protobuf_Payload *payload,
			const char *name,
			bool has_alias,
			uint64_t alias,
			uint64_t datatype,
			bool is_historical,
			bool is_transient,
			bool is_null,
			const void *value,
			size_t size_of_value) {

	int size = payload->metrics_count;
	if (size == 0) {
		payload->metrics = (org_eclipse_tahu_protobuf_Payload_Metric *) calloc(1, sizeof(org_eclipse_tahu_protobuf_Payload_Metric));
		if(payload->metrics == NULL) {
			fprintf(stderr, "Cannot allocate initial memory for data\n");
		} else {
			size = 1;
		}
	} else {
		size = grow_metrics_array(&payload->metrics, size, 1);
	}

	if (name == NULL) {
		DEBUG_PRINT(("Name is null"));
		payload->metrics[size-1].name = NULL;
	} else {
		payload->metrics[size-1].name = (char *)malloc((strlen(name)+1)*sizeof(char));
		strcpy(payload->metrics[size-1].name, name);
	}
	payload->metrics[size-1].has_alias = has_alias;
	if (has_alias) {
		payload->metrics[size-1].alias = alias;
	}
	payload->metrics[size-1].has_timestamp = true;
	payload->metrics[size-1].timestamp = get_current_timestamp();
	payload->metrics[size-1].has_datatype = true;
	payload->metrics[size-1].datatype = datatype;
	payload->metrics[size-1].has_is_historical = is_historical;
	if (is_historical) {
		payload->metrics[size-1].is_historical = is_historical;
	}
	payload->metrics[size-1].has_is_transient = is_transient;
	if (is_transient) {
		payload->metrics[size-1].is_transient = is_transient;
	}
	payload->metrics[size-1].has_is_null = is_null;
	if (is_null) {
		payload->metrics[size-1].is_null = is_null;
	}
	payload->metrics[size-1].has_metadata = false;
	payload->metrics[size-1].has_properties = false;

	// Default dynamically allocated members to NULL
	payload->metrics[size-1].value.string_value = NULL;

	DEBUG_PRINT(("Setting datatype and value - value size is %d\n", size_of_value));
	if (datatype == METRIC_DATA_TYPE_UNKNOWN) {
		fprintf(stderr, "Can't create metric with unknown datatype!\n");
	} else if (datatype == METRIC_DATA_TYPE_INT8) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int8_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
		payload->metrics[size-1].value.int_value = *((int8_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_INT16) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int16_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
		payload->metrics[size-1].value.int_value = *((int16_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_INT32) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int32_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
		payload->metrics[size-1].value.int_value = *((int32_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_INT64) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((int64_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
		payload->metrics[size-1].value.long_value = *((int64_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_UINT8) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %u\n", datatype, *((uint8_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
		payload->metrics[size-1].value.int_value = *((uint8_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_UINT16) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %u\n", datatype, *((uint16_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_int_value_tag;
		payload->metrics[size-1].value.int_value = *((uint16_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_UINT32) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %u\n", datatype, *((uint32_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
		payload->metrics[size-1].value.long_value = *((uint32_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_UINT64) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((uint64_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
		payload->metrics[size-1].value.long_value = *((uint64_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_DATETIME) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((uint64_t *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_long_value_tag;
		payload->metrics[size-1].value.long_value = *((uint64_t *)value);
	} else if (datatype == METRIC_DATA_TYPE_FLOAT) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %f\n", datatype, *((float *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_float_value_tag;
		payload->metrics[size-1].value.float_value = *((float *)value);
	} else if (datatype == METRIC_DATA_TYPE_DOUBLE) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %f\n", datatype, *((double *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_double_value_tag;
		payload->metrics[size-1].value.double_value = *((double *)value);
	} else if (datatype == METRIC_DATA_TYPE_BOOLEAN) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((bool *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_boolean_value_tag;
		payload->metrics[size-1].value.boolean_value = *((bool *)value);
	} else if (datatype == METRIC_DATA_TYPE_STRING || datatype == METRIC_DATA_TYPE_TEXT || datatype == METRIC_DATA_TYPE_UUID) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %s\n", datatype, (char *)value));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag;
		payload->metrics[size-1].value.string_value = (char *)malloc(size_of_value*sizeof(char));
		strcpy(payload->metrics[size-1].value.string_value, (char *)value);
	} else if (datatype == METRIC_DATA_TYPE_BYTES) {
		DEBUG_PRINT(("Datatype BYTES - Not yet supported\n"));
	} else if (datatype == METRIC_DATA_TYPE_DATASET) {
		DEBUG_PRINT(("Datatype DATASET - Not yet supported\n"));
	} else if (datatype == METRIC_DATA_TYPE_FILE) {
		DEBUG_PRINT(("Datatype FILE - Not yet supported\n"));
	} else if (datatype == METRIC_DATA_TYPE_TEMPLATE) {
		DEBUG_PRINT(("Setting datatype: %d, with value: %d\n", datatype, *((bool *)value)));
		payload->metrics[size-1].which_value = org_eclipse_tahu_protobuf_Payload_Metric_template_value_tag;
		payload->metrics[size-1].value.template_value = *((org_eclipse_tahu_protobuf_Payload_Template *)value);
	} else {
		DEBUG_PRINT(("Unknown datatype %u\n", datatype));
	}

	payload->metrics_count++;
	DEBUG_PRINT(("Size of metrics payload %d\n", payload->metrics_count));
}

/*
 * Encode a Payload into an array of bytes
 */
size_t encode_payload(uint8_t **buffer, size_t buffer_length, org_eclipse_tahu_protobuf_Payload *payload) {
        size_t message_length;
	bool node_status;

	// Create the stream
	pb_ostream_t node_stream = pb_ostream_from_buffer(*buffer, buffer_length);

	// Encode the payload
	DEBUG_PRINT(("Encoding...\n"));
	node_status = pb_encode(&node_stream, org_eclipse_tahu_protobuf_Payload_fields, payload);
	message_length = node_stream.bytes_written;
	DEBUG_PRINT(("Message length: %d\n", message_length));

        // Error Check
        if (!node_status) {
                fprintf(stderr, "Encoding failed: %s\n", PB_GET_ERROR(&node_stream));
                return -1;
        } else {
                DEBUG_PRINT(("Encoding succeeded\n"));
		return message_length;
        }
}

/*
 * Decode an array of bytes into a Payload
 */
bool decode_payload(org_eclipse_tahu_protobuf_Payload *payload, const void *binary_payload, int binary_payloadlen) {

	// Local vars for payload decoding
	bool status;
	pb_wire_type_t payload_wire_type;
	uint32_t payload_tag;
	bool payload_eof;
	const pb_field_t *payload_field;
	uint64_t udest64;
	int64_t dest64;
	pb_byte_t string_size[1];
	pb_byte_t *dest_pb_byte_t;

	pb_istream_t stream = pb_istream_from_buffer(binary_payload, binary_payloadlen);
	DEBUG_PRINT(("Bytes Remaining: %d\n", stream.bytes_left));

	// Loop over blocks while decoding portions of the payload
	while (pb_decode_tag(&stream, &payload_wire_type, &payload_tag, &payload_eof)) {
		DEBUG_PRINT(("payload_eof: %s\n", payload_eof ? "true" : "false"));
		DEBUG_PRINT(("\tBytes Remaining: %d\n", stream.bytes_left));
		DEBUG_PRINT(("\tWiretype: %d\n", payload_wire_type));
		DEBUG_PRINT(("\tTag: %d\n", payload_tag));

		if (payload_wire_type == PB_WT_VARINT) {
			for (payload_field = org_eclipse_tahu_protobuf_Payload_fields; payload_field->tag != 0; payload_field++) {
				if (payload_field->tag == payload_tag && (((payload_field->type & PB_LTYPE_VARINT) == PB_LTYPE_VARINT) ||
										((payload_field->type & PB_LTYPE_UVARINT) == PB_LTYPE_UVARINT))) {
					DEBUG_PRINT(("\tWire type is PB_WT_VARINT\n"));
					
					status = pb_decode_varint(&stream, &udest64);
					if (status) {
						DEBUG_PRINT(("\tVARINT - Success - new value: %ld\n", udest64));
					} else {
						fprintf(stderr, "\tVARINT - Failed to decode variant!\n");
						return false;
					}

					if (payload_field->tag == org_eclipse_tahu_protobuf_Payload_timestamp_tag) {
						payload->has_timestamp = true;
						payload->timestamp = udest64;
					} else if (payload_field->tag == org_eclipse_tahu_protobuf_Payload_seq_tag) {
						payload->has_seq = true;
						payload->seq = udest64;
					}
				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_SVARINT) == PB_LTYPE_SVARINT)) {
					DEBUG_PRINT(("\tWire type is PB_WT_SVARINT\n"));
					
					status = pb_decode_svarint(&stream, &dest64);
					if (status) {
						DEBUG_PRINT(("\tVARINT - Success - new value: %ld\n", dest64));
					} else {
						fprintf(stderr, "\tVARINT - Failed to decode variant!\n");
						return false;
					}
				}
			}
		} else if (payload_wire_type == PB_WT_64BIT) {
			DEBUG_PRINT(("\tWire type is PB_WT_64BIT\n"));
		} else if (payload_wire_type == PB_WT_STRING) {
			DEBUG_PRINT(("\tWire type is PB_WT_STRING\n"));
			for (payload_field = org_eclipse_tahu_protobuf_Payload_fields; payload_field->tag != 0; payload_field++) {
				if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_SUBMESSAGE) == PB_LTYPE_SUBMESSAGE)) {
					DEBUG_PRINT(("\tFound a PB_LTYPE_SUBMESSAGE\n"));

					// This is a metric!
					if (payload_field->ptr == NULL) {
						fprintf(stderr, "Invalid field descriptor\n");
						return false;
					}

					{
					org_eclipse_tahu_protobuf_Payload_Metric metric = org_eclipse_tahu_protobuf_Payload_Metric_init_zero;
					if(decode_metric(&metric, &stream)) {
						DEBUG_PRINT(("Decoding metric succeeded\n"));
						add_metric_to_payload(payload, &metric);
					} else {
						fprintf(stderr, "Decoding metric failed\n");
						return false;
					}
					}
				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_FIXED_LENGTH_BYTES) == PB_LTYPE_FIXED_LENGTH_BYTES)) {
					DEBUG_PRINT(("\tFound a PB_LTYPE_FIXED_LENGTH_BYTES\n"));
				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_STRING) == PB_LTYPE_STRING)) {
					DEBUG_PRINT(("\tFound a PB_LTYPE_STRING\n"));

					// Get the UUID
					//pb_byte_t string_size[1];
					status = pb_read(&stream, string_size, 1);
					if (status) {
						DEBUG_PRINT(("\t\tUUID Size: %d\n", string_size[0]));
					} else {
						fprintf(stderr, "\t\tFailed to read the UUID\n");
						return false;
					}

					//pb_byte_t dest[string_size[0]+1];
					dest_pb_byte_t = (pb_byte_t *)alloca(string_size[0]+1);
					status = pb_read(&stream, dest_pb_byte_t, string_size[0]);
					if (status) {
						dest_pb_byte_t[string_size[0]] = '\0';
						DEBUG_PRINT(("\t\tRead the UUID: %s\n", dest_pb_byte_t));
						payload->uuid = (char *)malloc((strlen(dest_pb_byte_t)+1)*sizeof(char));;
						strcpy(payload->uuid, dest_pb_byte_t);
					} else {
						fprintf(stderr, "\t\tFailed to read the UUID...\n");
						return false;
					}

				} else if (payload_field->tag == payload_tag && ((payload_field->type & PB_LTYPE_BYTES) == PB_LTYPE_BYTES)) {
					DEBUG_PRINT(("\tFound a PB_LTYPE_BYTES\n"));
				//} else {
					//DEBUG_PRINT(("\tother: %d\n", payload_field->type);
				}
			}
		} else if (payload_wire_type == PB_WT_32BIT) {
			DEBUG_PRINT(("\tWire type is PB_WT_32BIT\n"));
		} else {
			fprintf(stderr, "\tUnknown wiretype...\n");
		}
	}

#ifdef SPARKPLUG_DEBUG
	// Print the message data
	print_payload(payload);
#endif

	return true;
}

/*
 * Free memory from an existing Payload
 */
void free_payload(org_eclipse_tahu_protobuf_Payload *payload) {
	int i=0;
	for (i=0; i<payload->metrics_count; i++) {
		free(payload->metrics[i].name);
		// More TODO...
		// JPL 04/05/17... free up string data allocated memory
		if(  payload->metrics[i].which_value == 
			 org_eclipse_tahu_protobuf_Payload_Metric_string_value_tag ) // 15 ???
		{
		  if(payload->metrics[i].value.string_value)  // not null?
		  {
			free(payload->metrics[i].value.string_value);
		  }
		}
		
		if (payload->metrics[i].has_properties)
		{
			int j=0;
			for (j=0; j<payload->metrics[i].properties.keys_count; j++){
				free(payload->metrics[i].properties.keys[j]);
								
				if(payload->metrics[i].properties.values[j].which_value ==
					org_eclipse_tahu_protobuf_Payload_PropertyValue_string_value_tag) 
				{
					if(payload->metrics[i].properties.values[j].value.string_value)
					{	
						free(payload->metrics[i].properties.values[j].value.string_value);
					}
				}
			}
			free(payload->metrics[i].properties.keys);
			free(payload->metrics[i].properties.values);	
		}	
		
	}
}

#include <windows.h>
/*
 * Get the current timestamp in milliseconds
 */
uint64_t get_current_timestamp() {

#ifdef UNIX
	// Set the timestamp
	struct timespec ts;
	#ifdef __MACH__ // OS X does not have clock_gettime, use clock_get_time
		clock_serv_t cclock;
		mach_timespec_t mts;
		host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
		clock_get_time(cclock, &mts);
		mach_port_deallocate(mach_task_self(), cclock);
		ts.tv_sec = mts.tv_sec;
		ts.tv_nsec = mts.tv_nsec;
	#else
		clock_gettime(CLOCK_REALTIME, &ts);
	#endif
	return ts.tv_sec * UINT64_C(1000) + ts.tv_nsec / 1000000;

#else //UNIX
	FILETIME ft;
	ğtG‰ut;Pèay YTG‹MèDt$üƒæüƒ}ì ‰U‰Eğtf‹Qf‰U‹Uëf‹If‰M‹Mè9Utfƒ} v‹Uä+ÈJQPVè‹ ƒÄ‹Ef‹M+Çf‰P…TÿÿÿßPSèùŠ ‹}ä+uğƒÄÿ7wÿÔ`G ƒg jX‹Mô_^[d‰    ÉÂ ö©‹I u€©‹I é    hö@ è@t YÃV‹ñj èÆ§  ‹D$YN‹@™RPè@Ô ƒ¦Ğ    jX^Â ¸óæF èCx ì˜   ¡¨iI S‰Eì3À‰Eğ‰Eè‰EäVEäW‹}PEè‹ñPEğP‹G™^RP‹ËèÔ ÿuäE„‹ËPè²Ö ‹@ƒeü …Àu¸PfG jPM¬èÎ  M„ÆEüè©Vÿÿÿô`G ÆĞ   ‰E+=è  vhì  ÿuìÿu¼èj• ‹EƒÄ‰‹G‹Ë™RPÿw…\ÿÿÿÿuèPè.Ô ‹@ÆEü…Àu¸PfG jMÈ^VPè[  \ÿÿÿÆEüè3Vÿÿhí  ÿuìÿuØè• ÿuğè›¦  ƒÄ…Àtj^MÈÆEüè«  ƒMüÿM¬èŸ  ‹Mô‹Æ_^[d‰    ÉÂ jXÂ V‹5¨iI WjdèQ¦  Ç$ì  ¿ØGI VWè§” hí  VWè›” ƒÄjX_^Â ¸çF èÅv V‹u‹†  …Àu¸üeG PÿœcG ‰E‹$  ƒeü EPèœ  ÿu‹ğÿŒcG ‹Mô‹Æ^d‰    ÉÂ ¸çF èov ì(  SVÿu‹Ùèş  „Àtj ÿuÿuÿ aG éo  W3ö¹ÿ  3À½Î÷ÿÿf‰µÌ÷ÿÿó«f«…Ì÷ÿÿ‹ËPÿuèå  …Ì÷ÿÿ»Ô   …À…Ì÷ÿÿu¸ØGI P‹ÏèÑãÿÿ‹E‹ü   ;Æu¸ØGI Pè¹ãÿÿj,è*w Y‰E;Æ‰uüt‹ÏèYàÿÿ‹ ‹MjSPèñ   ë3À‰ƒ$  ƒMüÿEè‰uèPVSh}Î@ VVÿü`G ‹øj‰}ğèÚv Y‰8‹= dG hÿ   jÿVP‰Ejÿ×ƒø‰EuBjVVEÌVPÿ¤dG …Àt }Ğ2€  u‹‹$  èÓ  EÌPÿÔdG ëÍhÿ   jÿVÿujë´ÿuèWÀ ‹›$  Y;Ş_tKèh  Sè=À Yƒ}ÿu3Àë!Eì‰uìPÿuğÿø`G ÿuğÿ8cG 3À9uìÀ‹Mô^[d‰    ÉÂ ‹D$V‹ñWj ÿt$ƒ& ~‰F‹ÏÇG˜fG ÇGfG è8  ‹D$ÇŒfG Áà
‰F$ÆF(‹Æ_^Â V‹ñè   öD$tVè¥¿ Y‹Æ^Â é¾  SV‹t$2ÛVÿcG ƒø~fƒ>\u	fƒ~\uşÃŠÃ^[Â U‹ìƒìS‹]V3öjVSèÓz ƒÄÿuh,ªH SÿœdG ƒÄf93‹Ãtfƒ8\ufÇ / @@f90uîSÿcG ƒø~eW3À}îf‰uì««««f«‹ó}ì¥¥EìhªH P¥è(‰ YY_…ÀtOf‹ChªH f‰EøEìPè‰ Y…ÀYt3f‹Chø©H f‰EúEìPèïˆ Y…ÀYtÿuhªH hì©H SÿœdG ƒÄ^[ÉÂ ¸>çF èYs Qƒ} V‹ñW‰uğtÇF°fG ÇF¨fG ‹NF‹yøÿ@cG ‰ÿuƒeü ÿœcG ‰FjÆEüèat ‰FÇF   Ç¤fG ‹FY‹@ÿt0ÿDcG ‹MôÇ fG ‹Æ_^d‰    ÉÂ V‹ñÿvÇ¤fG èÿ½ öD$YtVèñ½ Y‹Æ^Â ‹‹@ÿ4ÿDcG ÃSUV‹ñW‹F^Ç fG ‹@|0ÿ@cG n‰ÿu ÿŒcG ‹ƒe  ‹@ÿt0ÿDcG ‹ÆN÷ØÀ#Á‹‹yøÿ@cG ‰ÿvÇ¤fG ès½ ‹ÆY÷ØÀ#Åÿ0ÿŒcG ÷Şö#ó‹‹@ÿ40ÿDcG _^][Ã¸^çF èr Qƒ} V‹ñW‰uğtÇFÄfG ÇF¼fG ‹NF‹yøÿ@cG ‰ÿuƒeü ÿœcG ‰FjÆEüès ‰FÇF   Ç¤fG ‹FY‹@ÿt0ÿDcG ‹MôÇ¸fG ‹Æ_^d‰    ÉÂ SUV‹ñW‹F^Ç¸fG ‹@|0ÿ@cG n‰ÿu ÿŒcG ‹ƒe  ‹@ÿt0ÿDcG ‹ÆN÷ØÀ#Á‹‹yøÿ@cG ‰ÿvÇ¤fG èT¼ ‹ÆY÷ØÀ#Åÿ0ÿŒcG ÷Şö#ó‹‹@ÿ40ÿDcG _^][ÃV‹ñèGşÿÿöD$tVè¼ Y‹Æ^Â V‹ñèJÿÿÿöD$tVèù» Y‹Æ^Â ö©‹I u€©‹I é    hö@ è_l YÃV‹5@cG W3ÿ‹D$ÿpÿøbG …Àu
ÿÖÿÖGƒÿd|å_3À^ÃV‹ñ‹…Étè   ƒ& ^ÃSV‹ñWÿvDè‡» ÿvH3Û‰^Dèz» ÿvL‰^Hèo» ‹œ   ¾œ   ƒÄ;Ë‰^Lt‹ÿP‰ÿ¶    ¾    èA» Y‰‹Îèûø _^[Ãÿt$è’   Â ¸pçF èão ƒìŠEV‹ñˆF(EØPè¯  ƒeü ‹ÈƒÀ÷ÙÉ#ÈQ‹Mè%   ƒMüÿMØè	ıÿÿÿu‹ÎèB   ‹Mô^d‰    ÉÂ V‹ñW‹|$‹;t…ÀtPÿŒcG ÿ7ÿ˜cG Pÿ7ÿcG ‰‹Æ_^Â ¸«çF èTo ¸Ø  èv SV‹ñ3ÛW‰uè9t
¸ÿÿ €éš  M„‰]à‰]À‰]È‰]Ì‰]Ğ‰]Ôèu	  ¹  3À½ïÿÿf‰ïÿÿó«ShªH jNS‰]üf«èV  …À¿ªH tSWjSNè@  …Àuÿv…ïÿÿhHªH PÿœdG ƒÄSWjSNè  ÷ØÀşÀˆEót?h¨   èêo ‹øY‰}Ø;ûÆEütSSSSh0u  h4ªH ‹Ïè7  ÇÌfG ë3ÿˆ]ü‰}äë5h°   è«o Y‰EØ;ÃÆEütSSSSh0u  h4ªH ‹Èèê  ë3Àˆ]ü‰Eä‹F‹}äïÿÿh @€QP‹Ïè<õ „Àu6‹=@cG ÿ×…Àÿ×‹øëÿ×‹øçÿÿ  Ï  €‹N‰}ÀUÀ‹RÿPéÆ   ‹‹ÏÿP‹N‰EÔ‹øUÀ‹Rÿ8]óu;ûvƒÿÿu¿TÕ€ëÀ‹ESM¤ÇE°˜fG ‹ ÇE¸fG Pè.úÿÿ¾ŒfG ‰u¤ƒìE¤‰e‹üSP‹ÏÆEüÇG˜fG ÇGfG è‡  M„‰7èÿ  ;ÃuT‹5@cG ÿÖ…ÀÿÖ‹ğëÿÖ‹ğæÿÿ  Î  €‹Eè‰uÀUÀ‹HR‹ÿPM¤ˆ]üèuúÿÿ‹şƒMüÿM„è©  ‹Çéi  E¤dÿÿÿPèI  ¾   ÆEüVˆ]òˆ]ˆ]óè,n ‹=@cG Y‰EÜ‰]ì8]…¶   8]ó…­   ‹MäEìPVÿuÜèDö ƒ}ìÿ”E8]uU9]ìv&EØSPÿuìÿuÜÿu„ÿcG …Àt‹EØëƒÈÿ;Ã”E8]u%‹EìUÀEĞ‹EèR‹H‹ÿPƒø•Eò9]ì”Eóë2ÿ×…Àÿ×ëÿ×%ÿÿ    €‰Eà‰EÀ‹EèUÀR‹H‹ÿPƒø•E8]ò„AÿÿÿÿuÜè@· ƒ}„ÿYt¡8cG ;Ãt	ÿu„ÿĞƒM„ÿ‹Mä‹ÿP8]òu8]udÿÿÿèS  ‹EèUÀR‹H‹ÿPdÿÿÿÆEüè:  M¤ˆ]üèùÿÿƒMüÿM„è<  ‹Eà‹Mô_^d‰    [ÉÂ Vq$VÿaG ‹^ÃSVW‹ù‹_$w$VÿaG ‹6ƒşu	‹Ïè†ñ ë…öu…ÿt	‹j‹ÏÿP_Cÿ^[Ã¸ÕçF è4k ƒìSV‹ñW‰uğèu  ~03Û‹Ï‰]üè  ÆEü‰^D‰^H‰^L‰^P‰^T‰^\‰^`‰^d‰^h‰^l‰^p‰^tÇFx   Ç†€      ÿ˜dG ‰   j0‰†„   ‰ˆ   ‰”   ˆ˜   èl Y‹È‰Mì;ËÆEütèï   ë3À‰†œ   ‹E‰F,‹E   ‹ÏPÆEüÿu‰    Ç}G ÿuÿuÿuè³î EäPÿaG H÷ØÀşÀˆ†˜   t‹Eä‰†   ‹Eè‰†”   ëÇ†   è  ‰”   ‹Mô‹Æ_^[d‰    ÉÂ V‹ñjƒf èVk …ÀYtÇ    ë3ÀN‰Fè¨å ƒf Ç}G ‹Æ^ÃÇ}G éèî V‹ñÇ}G èÚî öD$tVèğ´ Y‹Æ^Â ¸öçF è¥i QS‹aG VW3ÿ‹ñWWWW‰uğ‰~‰~ÿÓ‰FWWWW‰}üÿÓ‰FNÆEüè+å ‹Mô‰~‰~‰~ ‰~(ÇF$   ÇF,0u  ÇL}G ‹Æ_^[d‰    ÉÃ¸èF è2i QV‹ñW‰uğÇL}G ÿvÇEü   èO´ ‹F‹=8cG …ÀYt…ÿtPÿ×ƒf ‹F…Àt…ÿtPÿ×ƒf ‹Mô_^d‰    ÉÃ2ÀÂ V‹ñè“ÿÿÿöD$tVèü³ Y‹Æ^Â Ã°ÃV‹ñè   öD$tVèÜ³ Y‹Æ^Â ¸3èF è‘h QV‹ñ‰uğÇ}G ÇEü   èøÿÿ€eü N0Ç}G è}í ƒMüÿ‹Îèÿÿÿ‹Mô^d‰    ÉÃ¸RèF èDh QQSV‹ñW‰uğè,  ~03Û‹Ï‰]üˆ¨   ÇÜ|G è¿   ƒø…   ‹h†I ;Ëu)jèRi Y‹È‰Mì;ËÆEütèÛıÿÿë3À‹Èˆ]ü‰h†I 9Yu ‹E   Pÿuÿuÿuÿuèì ‹h†I ‹Ù;ût‹ÏèÃì S‹Ïè{   hl†I ÿaG Æ†¨   ë‹E‹Ï   PÿuÿuÿuÿuèÅë ‹E‹Mô‰F,‹Æ_^[d‰    ÉÂ U‹ìƒìƒeô W3À}ø«EüÇEü   PEôPj(j èçê ‹EøƒÄ_ÉÃ‹D$‹P‰Q‹P‰Q‹@P‰AÿaG Â ¸yèF èÿf ƒìSV‹ñ‰uğèAıÿÿ3ÛN0‰]üèÑüÿÿÆEü‰^D‰^H‰^L‰^P‰^T‰^\‰^`‰^d‰^h‰^l‰^p‰^tÇFx   Ç†€      ÿ˜dG ‰   j0‰†„   ‰ˆ   ‰”   ˆ˜   èÑg Y‹È‰Mì;ËÆEütè½üÿÿë3À‰†œ   EäP‰    Ç}G ÿaG …À•À:Ãˆ†˜   t‹Eä‰†   ‹Eè‰†”   ëÇ†   è  ‰”   ‹Mô‹Æ^[d‰    ÉÃV‹ñè   öD$tVè.± Y‹Æ^Â ÇÜ|G éGıÿÿƒÈÿÂ ‹A(…Àt‹L$‰°Â V‹ñè   öD$tVèï° Y‹Æ^Â ÇÌfG éıÿÿ¸ŒèF è™e QVW‹ù‰}ğƒÿƒeü wj ‹ÎÇF˜fG ÇFfG è  ‹Mô‹ÇÇŒfG _^d‰    ÉÃ¸ èF èPe QSUVW‹ñƒd$ ‹-8cG ÇD$   ~D$(‹ÏPèù  ‹Gj h€   jj jh   @Pÿ$cG ‹Ø‹;Ãtƒøÿt
…ítPÿÕƒÿ‰ƒ>ÿuƒ|$hô  ÿübG ÿD$ë¢ƒ>ÿ•ÀƒL$ÿL$(¶ğèòÿÿ‹L$‹Æ_^][d‰    ÉÂ Vj ÿt$‹ñÇF˜fG ÇFfG è±  ÇŒfG ÆF‹Æ^Â €a ÃV‹ñ€~ tVèÒóÿÿY‹ÎèÀñÿÿ^Ã¸´èF èUd QV‹ñ‰uğƒeü Nè¡ñÿÿ‹ƒùÿt¡8cG …ÀtQÿĞƒÿ‹Mô^d‰    ÉÃ¸ßèF èd ì\  SV… ıÿÿW3ÛP‹ñh  ‰]ìÿÀaG E¨NP‰Mğè÷   ‹@˜ûÿÿQSP… ıÿÿPÿaG M¨è"ñÿÿ…˜ûÿÿSPMĞÇEÜ˜fG ÇEäfG èBğÿÿ¿ŒfG ‰}Ğ8^(ÇEü   t`E¨MĞPè˜   ÿp‹5aG … ıÿÿPÿÖM¨èÊğÿÿ‹MğE¨SPè   ÿp… ıÿÿPÿÖM¨è©ğÿÿÿuàÿŒcG … ıÿÿPÿœcG ‰Eà‹uEĞSP‹ÎÇF˜fG ÇFfG è8  ‰>ÇEì   MĞˆ]üè_ğÿÿ‹Mô‹Æ_^[d‰    ÉÂ U‹ìƒì,VWEÔ3ÿ‹ñP‰}üè]   ‹@;Çt	Pÿ˜cG ‹øMÔèğÿÿ¡üfG ‹ÎPEüjPÇEü.   è  ‹üfG ;Át;Çr+ÇjPëjQ‹MWVè  ‹E_^ÉÂ ¸éF ènb ƒìLSVW‹=üfG j‰Mğ^3ÛVh`ªH MĞ‰]ìèæîÿÿ‹MğWP‰uüè’  MĞ‹øˆ]üèïÿÿ‹MğE¨SPèñ   ;=üfG ÇEü   t&9]¸tÿu¸ÿ˜cG ;ør‹MGVWSÿuğèy  ‰uìë‹ME¨VPèì  ‰uìM¨ˆ]üè1ïÿÿ‹Mô‹E_^[d‰    ÉÂ U‹ìƒì,SVEÔW3Û‹ñP‰]üè-ÿÿÿ‹@‹=˜cG ;ÃtPÿ×‹ØMÔèêîÿÿ¡üfG ‹ÎPEüjPÇEü.   è^  ‹üfG ;Át;Ãr€} t@jQë‹F…ÀtPÿ×jÿ5üfG ‹MPVèÂ  ‹E_^[ÉÂ ¸8éF è'a ƒì@SV‹ñ3ÛW‰uğ‰]ìèı   „Àtej¿`ªH [MĞSWè™íÿÿƒeü jP‹Îè  ƒMüÿMĞ‹ğè?îÿÿ;5üfG t&SWMĞèkíÿÿ‹MğFVP‰]üèØ  ƒMüÿMĞ‹ğèîÿÿSVj ÿuğëx‹Îè¼   ‹øƒÿÿ‰}ğujSë;ûju‹Ff‹@f-: f÷ØÀ$ş@@PSëC[M´Sh`ªH èíÿÿWP‹ÎÇEü   èn  ƒMüÿM´‹øè§íÿÿŠE„Àu+}ğöØÀS÷Ğ#EğWPV‹Mè°   ‹Mô‹E_^[d‰    ÉÂ V‹ñ‹F…Àt!Pÿ˜cG ƒør‹Ffƒ8\ufƒx\ujX^Ã3À^ÃVW3ÿ‹ñWhªH jWè<  …ÀujXëMWhªH jW‹Îè#  …ÀujëåWhø©H jW‹Îè  …ÀujëÍWhhªH jW‹Îèó  …Àt·3À9~•ÀH_^Ã¸TéF èo_ ƒì ƒ} V‹ñ‰uğtÇFgG ÇFgG j ‹Îèè   ‹U‹E‹Mƒeü R‰FPEÔP‰V‰N Ç gG è,  P‹ÎÆEüèè  €eü MÔèiìÿÿ‹Mô‹Æ^d‰    ÉÂ éSìÿÿƒ|$ VW‹|$‹ñt%…ÿÇFgG ÇFgG u3Àë
‹G‹@D8‹ ‰F$j W‹ÎèÚ   ‹G‰F‹G‰F‹G ‰F Ç gG ‹Æ_^Â V‹ñè–ÿÿÿöD$tVè¼© Y‹Æ^Â V‹ñ‹…Ét¡8cG …ÀtQÿĞƒ& ^Ã¸véF èW^ Qƒ} V‹ñW‰uğtÇF°fG ÇF¨fG ‹NF‹yøÿ@cG ‰ƒeü ƒf jÆEüèg_ ‰FÇF   Ç¤fG ‹FY‹@ÿt0ÿDcG ‹MôÇ fG ‹Æ_^d‰    ÉÂ ¸–éF èÒ] Qƒ} SV‹ñW‰uğtÇF°fG ÇF¨fG ‹F^‹xûÿ@cG ‰ƒeü ƒf jÆEü_Wèß^ ‰F‰~Ç¤fG ‹F‹=DcG Y‹@ÿt0ÿ×‹EÇ fG ÿpÿ˜cG P‹EÿpÿcG ‰F‹‹@ÿt0ÿ×‹Mô‹Æ_^[d‰    ÉÂ V‹ñW‹|$‹F;Gt!…ÀtPÿŒcG ÿwÿ˜cG PÿwÿcG ‰F‹Æ_^Â U‹ìQƒeü V‹ñ‹F…ÀtPÿ˜cG ‹U;üfG u‹Më‹MW<;ø_v+Á‹Ğ‹FjH‹MPRè   ‹E^ÉÂ ¸¶éF è™\ Qƒ} V‹ñW‰uğtÇF°fG ÇF¨fG ‹NF‹yøÿ@cG ‰ÿuƒeü ÿuÿcG ‰FjÆEüè] ‰FÇF   Ç¤fG ‹FY‹@ÿt0ÿDcG ‹MôÇ fG ‹Æ_^d‰    ÉÂ U‹ìVW‹ùƒ tI‹G…ÀtPÿ˜cG ‹M‹üfG ;Êt;Ás‹È;Mr2+M‹ñxÿu‹GÿupPè4  ƒÄ…ÀtNyä¡üfG _^]Â ‹Æëö‹ÂëòVW‹ùƒ t@‹t$;5üfG u‹G…ÀtPÿ˜cG pÿ…ö|‹G‹L$j jpPè   ;üfG uNyá¡üfG _^Â ‹Æë÷U‹ìQSVW‹ùƒ tK‹G…Àu!Eüë
Pÿ˜cG ‰Eü‹E‹]‹ğÃ;Eüw&‰E‹GSÿupPèx  ƒÄ…ÀtFÿE‹E;Eüvİ¡üfG _^[ÉÂ ‹ÆëõSV‹ñ3ÛW9^t;‹F‹|$;Ãt	Pÿ˜cG ‹Ø;û}#‹F‹L$j jxPè^ÿÿÿ;üfG uGëİ‹Çë¡üfG _^[Â U‹ìQQS‹]W‹ù3É;Ùu3À9O•ÀéŞ   9OuƒÈÿéÑ   f9Ct
f‹@@f…Òuö+ÃVÑøH‰Eø‹ğ‹G+u;Áu3ÀëPÿ˜cG ‹M+Á‰E‹E;Es‰E‹E;ğs‹ÆP‹ECP‹GHPèu   ƒÄ‰Eü…Àua‹E+Æƒøÿ‰Eüu‹Eğ‹EøHÿ;ñrEfƒ|Cş ë7ƒøu8‹G…ÀtPÿ˜cG ‹M‹UÊH;Èr‹G…ÀtPÿ˜cG ‹Ofƒ|Aş uƒeü ‹Eü^_[ÉÂ U‹ìSVWè¬   ƒøuÿuÿuÿuÿuPh   ÿ aG é€   ‹u‹= cG 3Û9]u‰]ë+D6ƒÀ$üè‡` ‹ÄSL6SQPV‰EÿuˆSSÿ×‹E‰E9]t(D6ƒÀ$üèW` ‹ÄSL6SQPV‰EÿuˆSSÿ×‹]VSVÿujh   ÿaG eôHH_^[]ÃU‹ìì  €=0HI  u3…ìşÿÿÇ…ìşÿÿ  Pÿ$aG …Àt3Àƒ½üşÿÿ”À£,HI Æ0HI ¡,HI ÉÃ‹Á‹L$‰ˆ  ‹L$‰ˆ  Ç gG Â V‹ñè   öD$tVè–£ Y‹Æ^Â ÇgG Ã¸ÈéF èDX V‹ñÿuFPÿÀbG ‹†  ‰Ej ƒeü jVhÓñ@ Mÿuh  è    ƒøÿ^t	…ÀtjXë3À‹Môd‰    ÉÂ Ã¸ìéF èãW ƒì\öEV‹ñu‹€x u€x t‹Eéô   ‹jÿuMèÿp èºÚÿÿƒeü öE…”   ¸xªH ÇEÀ(fG ‹ÈÇEà fG …Éu¸ØGI SWMj QPMÀè#Oÿÿ‹EÈ»PfG …ÀÆEü‰]t‰E‹f‹AHPè7  ‹‹øE˜Pè…   ‹@ÆEü…Àu‹ÃjMèÿuWPè€ÜÿÿM˜ÆEüèó5ÿÿ€eü MÀèç5ÿÿ_[‹f‹AHPè3  ÿuˆE‹Mèÿu‹p ÿuÿuèÚÿÿPVÿ”dG ƒMüÿMè‹ğèÛÚÿÿ‹Æ‹Mô^d‰    ÉÂ U‹ìQf‹AHƒeü Pÿuè   ‹EÉÂ ¸XêF è˜V ìà   ƒeè SVW‰MìEò¿(fG ¾ fG j PM´‰}´‰uÔèMÿÿ·Ej[PE´hÌªH P‰]üèú  E´hÀªH P…ÿÿÿPè‡  ƒÄ‰E‹Mì…dÿÿÿPÆEüèz  j <ÿÿÿj Q‹ÈÆEüè’PÿÿÿuMŒÆEüQ‹Èè  <ÿÿÿÆEüèÄ4ÿÿdÿÿÿÆEüèµ4ÿÿÿÿÿÆEüè¦4ÿÿƒì(¸xªH ‹Ì‹Ğ…Ò‰eì‰9‰q u¸ØGI Uj RPèVMÿÿƒì(Eñ‹Ì‰eÜSPh¬ªH ÆEü	è³=ÿÿƒì(Eó‹Ì‰eäSPh”ªH ÆEü
è˜=ÿÿƒì(EŒ‹Ì‰eàSPÆEüè›‰ÿÿÿuÆEüèZ Ä¤   ‰]èMŒˆ]üè4ÿÿ€eü M´è4ÿÿ‹Mô‹E_^d‰    [ÉÂ ¸¥êF èU ìØ   SV3ÛW‹ñEóSPM¼ÇE¼(fG ÇEÜ fG èKÿÿ·EPE¼hÌªH P‰]üèz
  E¼hÀªH P…ÿÿÿPè  ƒÄ‹ø…lÿÿÿ‹ÎPÆEüèü   SDÿÿÿSQ‹ÈÆEüèOÿÿM”WQ‹ÈÆEüèŸ	  DÿÿÿÆEüèJ3ÿÿlÿÿÿÆEüè;3ÿÿÿÿÿÆEüè,3ÿÿjEƒì(‹Ì‰eèjPhÜªH èc<ÿÿƒì(Eò‹Ì‰eäjPh”ªH ÆEüèG<ÿÿƒì(E”‹Ì‰eìjPÆEü	èIˆÿÿÆEüè ƒÄ|M”‹ğˆ]üèÆ2ÿÿƒMüÿM¼èº2ÿÿ‹Môf‹Æ_^d‰    [ÉÂ U‹ìQEV‹qtPEüƒÁpPè–  3À9uü^•ÀÉÂ U‹ìQƒeü V‹uÁ  j Q‹ÎÇ(fG ÇF  fG èJÿÿ‹Æ^ÉÂ ¸ÀêF èoS ƒìTS3ÛEóSPMÈÇEÈ(fG ÇEè fG è÷Iÿÿ‹E‰]üHH„  -  t6H…  ‹EHt
ƒèeuèAn jÿuÿ„dG ƒMüÿMÈèã1ÿÿjXéü   W‹}h4HI Wèûo ‹EY£8HI Y‹€  ‰EE SPMÆEüèØ   ‹@;Ãu¸PfG VPWÿˆdG M è‹1ÿÿÿ54HI ¾  ˆ]üVWèvp ¡8HI VƒÀWPèOp ‹5ŒdG ƒÄjWÿÖjfW‰EÿÖ‹8HI ^S9™  tÿuÿdG ÿ54HI h°  jfëPÿdG ÿ54HI h  jWè%p ƒMüÿƒÄMÈèÿ0ÿÿjX_ëÿ54HI ÿÀ`G ƒMüÿMÈèá0ÿÿ3À‹Mô[d‰    ÉÂ ¸!ëF èîQ ì¨   SV‹ñ…tÿÿÿW3Û‹P‰]ìèo  jEó_MÄSP‰}üÇEÄ(fG ÇEä fG èXHÿÿ8]ÆEüu=9]€t8‹…|ÿÿÿ;Ãu¸PfG ‹SSPEœh_  Pè«  PMÄÆEüèÄ0ÿÿÆEüMœë$‹Eœh  Pèş   PMÄÆEüè0ÿÿÆEüMœè0ÿÿ9]Ğt5EœSP‹ÎèÎ  SPMÄÆEüèÿcÿÿ;0fG MœÆEü”Eèã/ÿÿ8]t_…LÿÿÿWP‹Îè–  PEœhp£H PÆEüèÉÑÿÿƒÄÿ50fG ‹ÈƒÀÆEü÷ÙÉS#ÈQMÈè¼ÿÿMœÆEüè/ÿÿLÿÿÿÆEüè/ÿÿ‹uEÄSP‹ÎÇ(fG ÇF  fG è©Gÿÿ‰}ìMÄÆEüèT/ÿÿtÿÿÿˆ]üèF/ÿÿ‹Mô‹Æ_^[d‰    ÉÂ ¸¸ëF èQP ì  SV3ÛW‰MìEã¿(fG ¾ fG SPM„‰]Ü‰}„‰u¤èÌFÿÿ‹EìÇEü   ·@HPE„hÌªH Pè¯  ƒÄEë\ÿÿÿ‰½\ÿÿÿSP‰µ|ÿÿÿèFÿÿÿu…\ÿÿÿÆEühğªH Pèx  E„hÀªH P…äşÿÿPè
  ƒÄ‰Eä‹Mì…”şÿÿPÆEüèøûÿÿSÿÿÿSQ‹ÈÆEüèJÿÿÿuä4ÿÿÿÆEüQ‹Èè–  ÿÿÿÆEü	èA.ÿÿ”şÿÿÆEüè2.ÿÿäşÿÿÆEüè#.ÿÿàıÿÿèİ EâSPM¬ÆEü
‰}¬‰uÌèÒEÿÿSEóƒì(ÆEü‹Ì‰eäjPhì¡H è57ÿÿƒì(…4ÿÿÿ‹Ì‰eØjPÆEüè4ƒÿÿàıÿÿÆEüèÚ“ :Ãt{ƒì(Eó‹Ì‰eØjPhÜGI è–Hÿÿƒì(…\ÿÿÿ‹Ì‰eäjPÆEüèï‚ÿÿƒì(E„‹Ì‰eÔjPÆEüèØ‚ÿÿ…¼şÿÿàıÿÿPÆEüèo PM¬ÆEüèÅ-ÿÿ¼şÿÿÆEüè<-ÿÿ9]¸u‹Eìÿp f‹@HPE¬ÿuPèI ƒÄ‹ME¬SP‰9‰q èQEÿÿÇEÜ   M¬ÆEü
èø,ÿÿàıÿÿÆEüè”’ 4ÿÿÿÆEüèÚ,ÿÿ\ÿÿÿÆEüèË,ÿÿM„ˆ]üèÀ,ÿÿ‹Mô‹E_^d‰    [ÉÂ ¸ëëF èÊM ƒìXSVWEœ‰eğÿu3ÛP‰]ìèUıÿÿ9]¨j_‰}üu)‹uESPhØGI ‹ÎÇ(fG ÇF  fG è5Eÿÿ‰}ìéı   ESPMÄÇEÄ(fG ÇEä fG è
Dÿÿ‹UÆEü;Óu#9]…œ   9]…“   EœMÄPèŠ,ÿÿé‚   ‹M;Ëu%9]u‹E¤;Ãu¸PfG RPEÄPè¹  ƒÄëZ;ËtV9]u‹E¤;Ãu¸PfG QRPEÄPè“  ƒÄë4;Ët09]t+‹E¤;Ãu¸PfG ÿuQRPEÄPèj  ƒÄë¸–ø@ Ãj3Û_‹uEÄSP‹ÎÇEü   Ç(fG ÇF  fG è¬CÿÿMÄ‰}ìÆEüèW+ÿÿMœˆ]üèL+ÿÿ‹Mô‹Æ_^d‰    [ÉÂ U‹ìQÿuƒeü è   ‹EÉÂ ¸ìF è@L ƒì0ƒeğ S‹ÙVWƒ{Du'‹uÃ¨  j S‹ÎÇ(fG ÇF  fG è,Cÿÿ‹Æé•   ƒì(ƒH  ‰eì‹Ì¿(fG ¾ fG j P‰9‰q è CÿÿEÄPèQ> ƒÄ,3ÀÇEü   9EĞu-‹MÃ¨  PS‰9‰q èÑBÿÿ€eü MÄÇEğ   èx*ÿÿ‹Eë)‹]PEÄ‹ËP‰;‰s è¥Bÿÿ€eü MÄÇEğ   èL*ÿÿ‹Ã‹Mô_^d‰    [ÉÂ U‹ìƒì3ÀÆEäI8E‰EüÆEånÆEæsÆEçtÆEèaÆEélÆEêlÆEëSÆEìhÆEíiÆEîeÆEïlÆEğdˆEñtÆEñ ÆEòWÆEóiÆEôzÆEõaÆEörÆE÷dˆEø‹MEjPEäPè¸Dÿÿ‹EÉÂ ¸CìF èØJ ƒì0ƒeğ SVWj »(fG ¿ fG QMÄ‰]Ä‰}äèÑAÿÿjXj\‰Eüƒì(‹Ì‰eìPÿuèÜ~ÿÿMÄèN   ‹uEÄj P‹Î‰‰~ è›AÿÿÇEğ   €eü MÄèB)ÿÿ‹Mô‹Æ_^d‰    [ÉÂ ‹L$D$Pÿt$èÜ   Ã¸`ìF è:J ƒì,SV‹ñ3Û9]‰]üt`9^uEPèm)ÿÿëPSMÿu0èrÿÿˆEó‹FH‹ÎPÿu0èrÿÿ8]óuI:Ãuÿu0Njèğ\ÿÿÿ50fG E÷ØÀM#ÁSPNèµÿÿƒMüÿMè•(ÿÿ‹Mô^[d‰    ÉÂ, :ÃtÄÿ50fG EÈMjPèæCÿÿÿ50fG ‹ÈƒÀÆEü÷ÙÉS#ÈQNè¸´ÿÿMÈˆ]üèC(ÿÿë ¸tìF è_I ƒì ‹Eƒeì SVW‹}‰Mä‰Eàfƒ? „Ğ  fƒ?%…¼  j[ûf‹f=% „ª  3öf;Æ‰uğtHf=# u]ìë-f=* uƒE‹E‹@ü‰Eğëf=- tf=+ tf=0 tf=  uf‹ûf;Æu½9uğu!WèZ Y‰Eğf‹f…ÀtPèPY …ÀYtûëé3Ûfƒ?.uGGfƒ?*u0ƒEG‹EG‹Xüƒeè jhä©H Wè X ƒÄ…Àu,ƒÇÇEè   ëMWè²Y Y‹Øf‹f…ÀtËPèğX …ÀYtÀGGëé·ƒèFt#ƒètHHtƒètƒèuÇEè   ëÇEè   GG·Eè¹c  ;Á€   „®   ƒèC„¥   jY+Á„ƒ   +Á„’   +Átw-Ğÿ  „ƒ   +Á„„   ·ƒøi  „7  ƒèG„ø   ƒè„%  ƒè„  H„ß   H„   H„Ñ   é  -s  t6-Ğÿ  t&ƒèt
ƒètƒèuœƒE‹E‹@ü…Àt Pè‡G Yë#ƒEj^ë,ƒE‹E‹@ü…Àuj^ëPÿ<cG ‹ğƒş}j^…ö„Pÿÿÿ…Ût;ó|‹ó;uğ–   ‹uğé   ƒ>  9Eğ~‹EğÀƒÀ$üè6N ƒE‹ô‹EQQƒÃİ@øİ$SÿuğhØ©H Vè}V VèñF ƒÄë2ƒE¾€   ëƒènt2Ht#Htƒètƒètë$ƒEj ^‹EğÃ;Æ|‹ğëöEêtæƒEëäƒEuìëÿEìGGé&ıÿÿÿuì‹MäEÔPèÁ.ÿÿ‹ƒeü Æ@èÇ>ÿÿÿuà‹ ÿuPèƒU ƒMüÿƒÄMÔè§/ÿÿ‹MôeÈd‰    _^[ÉÂ U‹ìQVW‹}‹ñWè+   ‹v‰E;Ætf‹f;HrEë‰uüEü‹‹E_^‰ÉÂ ‹A‹$HI ‹H;ÊtV‹t$f‹6f9qs‹Ië‹Á‹	;Êuí^Â ¸§ìF è«E ƒìTƒeğ SVWj »(fG ÿu¿ fG MÈ‰]È‰}èè¢<ÿÿj^ÿu‰uüèE YPMÌÿuèÛ   VPM èI   ‹uE j P‹ÎÆEü‰‰~ èd<ÿÿÇEğ   M ÆEüè$ÿÿ€eü MÈèÿ#ÿÿ‹Mô‹Æ_^[d‰    ÉÃ¸¼ìF èE QSV3Û‹ñ9]W‰uğtÇHfG ÇF @fG ‹‹xşÿ@cG ‰‹E~S‹ÏŠ ‰]üˆè]>ÿÿÿ50fG ‹ÏSÿuè.Xÿÿ‰^‰^‰^‹F ‹@ÿt0 ÿDcG ‹Mô‹Æ_^[d‰    ÉÂ ¡0fG S‹\$V‹ñW+F;Ãwè¹ …Ûv8‹ûj ~‹ÎWèkaÿÿ„Àt%‹F‹V‹L$Bf‹f‰@@AAKuó‹F‰~fƒ$x ‹Æ_^[Â ö©‹I u€©‹I é    hö@ è×? YÃV‹ñhh«H ÿ(aG ‰‹Æ^Ã‹…ÀtPÿ,aG Ã¸ÈìF èãC ƒì‹SV3öW;Æ‰eğ‰uü
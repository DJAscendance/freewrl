#include <stddef.h>
#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <string.h>
#include "DIS.h"
#define NULL ((void *)0)
#define FALSE 0
#define TRUE 1

enum {
  UNSET,
  PRIMITIVE,
  CLASSREF,
  FIXED_LIST,
  VARIABLE_LIST,
};
static int TYPE_SIZE [] = {
  (int)sizeof(unsigned int),
  (int)sizeof(unsigned char),
  (int)sizeof(char),
  (int)sizeof(double),
  (int)sizeof(short),
  (int)sizeof(unsigned long long),
  (int)sizeof(float),
  (int)sizeof(unsigned short),
  (int)sizeof(int),
  (int)sizeof(long long),
  (int)sizeof(struct SystemID),
  (int)sizeof(struct RadioEntityType),
  (int)sizeof(struct LayerHeader),
  (int)sizeof(struct AcousticEmitterSystem),
  (int)sizeof(struct FourByteChunk),
  (int)sizeof(struct Orientation),
  (int)sizeof(struct OneByteChunk),
  (int)sizeof(struct EventID),
  (int)sizeof(struct VectoringNozzleSystemData),
  (int)sizeof(struct ObjectType),
  (int)sizeof(struct FundamentalParameterDataIff),
  (int)sizeof(struct EightByteChunk),
  (int)sizeof(struct FixedDatum),
  (int)sizeof(struct GridAxisRecord),
  (int)sizeof(struct AggregateID),
  (int)sizeof(struct TwoByteChunk),
  (int)sizeof(struct ClockTime),
  (int)sizeof(struct Relationship),
  (int)sizeof(struct Vector3Float),
  (int)sizeof(struct ModulationType),
  (int)sizeof(struct SimulationAddress),
  (int)sizeof(struct IffFundamentalData),
  (int)sizeof(struct AggregateType),
  (int)sizeof(struct BeamData),
  (int)sizeof(struct NamedLocation),
  (int)sizeof(struct RecordSet),
  (int)sizeof(struct SphericalHarmonicAntennaPattern),
  (int)sizeof(struct ShaftRPMs),
  (int)sizeof(struct IntercomCommunicationsParameters),
  (int)sizeof(struct AcousticBeamFundamentalParameter),
  (int)sizeof(struct EntityType),
  (int)sizeof(struct FundamentalParameterData),
  (int)sizeof(struct ApaData),
  (int)sizeof(struct Environment),
  (int)sizeof(struct AcousticEmitter),
  (int)sizeof(struct AngularVelocityVector),
  (int)sizeof(struct AggregateMarking),
  (int)sizeof(struct EntityID),
  (int)sizeof(struct SixByteChunk),
  (int)sizeof(struct Vector3Double),
  (int)sizeof(struct Pdu),
  (int)sizeof(struct VariableDatum),
  (int)sizeof(struct ArticulationParameter),
  (int)sizeof(struct Marking),
  (int)sizeof(struct Point),
  (int)sizeof(struct PropulsionSystemData),
  (int)sizeof(struct EmitterSystem),
  (int)sizeof(struct PduContainer),
  (int)sizeof(struct ElectronicEmissionBeamData),
  (int)sizeof(struct LogisticsFamilyPdu),
  (int)sizeof(struct ServiceRequestPdu),
  (int)sizeof(struct RepairCompletePdu),
  (int)sizeof(struct DeadReckoningParameter),
  (int)sizeof(struct BeamAntennaPattern),
  (int)sizeof(struct SyntheticEnvironmentFamilyPdu),
  (int)sizeof(struct AcousticEmitterSystemData),
  (int)sizeof(struct RepairResponsePdu),
  (int)sizeof(struct SimulationManagementFamilyPdu),
  (int)sizeof(struct AntennaLocation),
  (int)sizeof(struct DataQueryPdu),
  (int)sizeof(struct BurstDescriptor),
  (int)sizeof(struct LinearObjectStatePdu),
  (int)sizeof(struct CreateEntityPdu),
  (int)sizeof(struct RadioCommunicationsFamilyPdu),
  (int)sizeof(struct AcousticBeamData),
  (int)sizeof(struct IntercomSignalPdu),
  (int)sizeof(struct GridAxisRecordRepresentation2),
  (int)sizeof(struct LinearSegmentParameter),
  (int)sizeof(struct GridAxisRecordRepresentation1),
  (int)sizeof(struct GridAxisRecordRepresentation0),
  (int)sizeof(struct RemoveEntityPdu),
  (int)sizeof(struct ResupplyReceivedPdu),
  (int)sizeof(struct WarfareFamilyPdu),
  (int)sizeof(struct ElectronicEmissionSystemData),
  (int)sizeof(struct ActionRequestPdu),
  (int)sizeof(struct SupplyQuantity),
  (int)sizeof(struct AcknowledgePdu),
  (int)sizeof(struct DistributedEmissionsFamilyPdu),
  (int)sizeof(struct IffAtcNavAidsLayer1Pdu),
  (int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu),
  (int)sizeof(struct ActionRequestReliablePdu),
  (int)sizeof(struct DesignatorPdu),
  (int)sizeof(struct GriddedDataPdu),
  (int)sizeof(struct SetRecordReliablePdu),
  (int)sizeof(struct StopFreezePdu),
  (int)sizeof(struct ResupplyCancelPdu),
  (int)sizeof(struct EntityManagementFamilyPdu),
  (int)sizeof(struct StartResumePdu),
  (int)sizeof(struct TransmitterPdu),
  (int)sizeof(struct TrackJamTarget),
  (int)sizeof(struct ElectronicEmissionsPdu),
  (int)sizeof(struct ResupplyOfferPdu),
  (int)sizeof(struct MinefieldFamilyPdu),
  (int)sizeof(struct SetDataReliablePdu),
  (int)sizeof(struct EventReportPdu),
  (int)sizeof(struct PointObjectStatePdu),
  (int)sizeof(struct EnvironmentalProcessPdu),
  (int)sizeof(struct DataPdu),
  (int)sizeof(struct IsGroupOfPdu),
  (int)sizeof(struct MinefieldDataPdu),
  (int)sizeof(struct TransferControlRequestPdu),
  (int)sizeof(struct EntityInformationFamilyPdu),
  (int)sizeof(struct AcknowledgeReliablePdu),
  (int)sizeof(struct StartResumeReliablePdu),
  (int)sizeof(struct IffAtcNavAidsLayer2Pdu),
  (int)sizeof(struct ArealObjectStatePdu),
  (int)sizeof(struct DataQueryReliablePdu),
  (int)sizeof(struct AggregateStatePdu),
  (int)sizeof(struct EntityStateUpdatePdu),
  (int)sizeof(struct MinefieldStatePdu),
  (int)sizeof(struct DataReliablePdu),
  (int)sizeof(struct CommentPdu),
  (int)sizeof(struct CommentReliablePdu),
  (int)sizeof(struct DetonationPdu),
  (int)sizeof(struct SetDataPdu),
  (int)sizeof(struct RecordQueryReliablePdu),
  (int)sizeof(struct CollisionPdu),
  (int)sizeof(struct ActionResponsePdu),
  (int)sizeof(struct FirePdu),
  (int)sizeof(struct ReceiverPdu),
  (int)sizeof(struct UaPdu),
  (int)sizeof(struct IntercomControlPdu),
  (int)sizeof(struct SignalPdu),
  (int)sizeof(struct RemoveEntityReliablePdu),
  (int)sizeof(struct SeesPdu),
  (int)sizeof(struct CreateEntityReliablePdu),
  (int)sizeof(struct StopFreezeReliablePdu),
  (int)sizeof(struct EventReportReliablePdu),
  (int)sizeof(struct MinefieldResponseNackPdu),
  (int)sizeof(struct CollisionElasticPdu),
  (int)sizeof(struct ActionResponseReliablePdu),
  (int)sizeof(struct IsPartOfPdu),
  (int)sizeof(struct MinefieldQueryPdu),
  (int)sizeof(struct EntityStatePdu),
  (int)sizeof(struct FastEntityStatePdu),
};

struct disfieldattr {
    int kind; //A UNSET
    int type; //B PT_SHORT, type_...
    char *name; //C ivar name
    char *comment; //D
    int listLength; //E if this is a list, the length (if variable, extracted from other field called countField)
    char *countfieldname;  //F if variable_list, where get the count
    int countfieldindex; //F2 do I need a numerical offset to variable_list count field?
    //int isdynamiclistlengthField, //they use it for code generation to tinker with getter/setter
    int dynamicListClassAttribute; //G the underyling type of the list this counter belongs to 
    double defaultvalue;  //H if primitive, default. I put void*, so consistent size for generic value
    int listkind; //I instead of underlyintTypeIsPrimitive, underlyingTypeIsClass
    int couldBeString; //J
    int isBitField; //K
    //list bitfieldlist
    //int shouldSerialize - no case of serialize='false' in DIS2012.xml
    int size; //L in bytes
    int offset; //M in bytes from start of owning struct
};
struct disfieldattr FIELDS_SystemID [] = {
  {PRIMITIVE, type_USHORT, "systemType", "System Type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SystemID,systemType), },
  {PRIMITIVE, type_USHORT, "systemName", "System name, an enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SystemID,systemName), },
  {PRIMITIVE, type_UBYTE, "systemMode", "System mode", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct SystemID,systemMode), },
  {PRIMITIVE, type_UBYTE, "changeOptions", "Change Options", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct SystemID,changeOptions), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RadioEntityType [] = {
  {PRIMITIVE, type_UBYTE, "entityKind", "Kind of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RadioEntityType,entityKind), },
  {PRIMITIVE, type_UBYTE, "domain", "Domain of entity (air, surface, subsurface, space, etc)", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RadioEntityType,domain), },
  {PRIMITIVE, type_USHORT, "country", "country to which the design of the entity is attributed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RadioEntityType,country), },
  {PRIMITIVE, type_UBYTE, "category", "category of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RadioEntityType,category), },
  {PRIMITIVE, type_UBYTE, "nomenclatureVersion", "specific info based on subcategory field", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RadioEntityType,nomenclatureVersion), },
  {PRIMITIVE, type_USHORT, "nomenclature", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RadioEntityType,nomenclature), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_LayerHeader [] = {
  {PRIMITIVE, type_UBYTE, "layerNumber", "Layer number", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct LayerHeader,layerNumber), },
  {PRIMITIVE, type_UBYTE, "layerSpecificInformaiton", "Layer speccific information enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct LayerHeader,layerSpecificInformaiton), },
  {PRIMITIVE, type_USHORT, "length", "information length", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct LayerHeader,length), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AcousticEmitterSystem [] = {
  {PRIMITIVE, type_USHORT, "acousticName", "This field shall specify the system for a particular UA emitter.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcousticEmitterSystem,acousticName), },
  {PRIMITIVE, type_UBYTE, "acousticFunction", "This field shall describe the function of the acoustic system. ", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AcousticEmitterSystem,acousticFunction), },
  {PRIMITIVE, type_UBYTE, "acousticID", "This field shall specify the UA emitter identification number relative to a specific system. This field shall be represented by an 8-bit unsigned integer. This field allows the differentiation of multiple systems on an entity, even if in some instances two or more of the systems may be identical UA emitter types. Numbering of systems shall begin with the value 1. ", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AcousticEmitterSystem,acousticID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_FourByteChunk [] = {
  {FIXED_LIST, type_BYTE, "otherParameters", "four bytes of arbitrary data", 4, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct FourByteChunk,otherParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Orientation [] = {
  {PRIMITIVE, type_FLOAT, "psi", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Orientation,psi), },
  {PRIMITIVE, type_FLOAT, "theta", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Orientation,theta), },
  {PRIMITIVE, type_FLOAT, "phi", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Orientation,phi), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_OneByteChunk [] = {
  {FIXED_LIST, type_BYTE, "otherParameters", "one byte of arbitrary data", 1, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct OneByteChunk,otherParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EventID [] = {
  {PRIMITIVE, type_USHORT, "site", "The site ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EventID,site), },
  {PRIMITIVE, type_USHORT, "application", "The application ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EventID,application), },
  {PRIMITIVE, type_USHORT, "eventNumber", "the number of the event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EventID,eventNumber), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_VectoringNozzleSystemData [] = {
  {PRIMITIVE, type_FLOAT, "horizontalDeflectionAngle", "horizontal deflection angle", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct VectoringNozzleSystemData,horizontalDeflectionAngle), },
  {PRIMITIVE, type_FLOAT, "verticalDeflectionAngle", "vertical deflection angle", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct VectoringNozzleSystemData,verticalDeflectionAngle), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ObjectType [] = {
  {PRIMITIVE, type_UBYTE, "entityKind", "Kind of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ObjectType,entityKind), },
  {PRIMITIVE, type_UBYTE, "domain", "Domain of entity (air, surface, subsurface, space, etc)", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ObjectType,domain), },
  {PRIMITIVE, type_USHORT, "country", "country to which the design of the entity is attributed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ObjectType,country), },
  {PRIMITIVE, type_UBYTE, "category", "category of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ObjectType,category), },
  {PRIMITIVE, type_UBYTE, "subcategory", "subcategory of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ObjectType,subcategory), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_FundamentalParameterDataIff [] = {
  {PRIMITIVE, type_FLOAT, "erp", "ERP", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterDataIff,erp), },
  {PRIMITIVE, type_FLOAT, "frequency", "frequency", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterDataIff,frequency), },
  {PRIMITIVE, type_FLOAT, "pgrf", "pgrf", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterDataIff,pgrf), },
  {PRIMITIVE, type_FLOAT, "pulseWidth", "Pulse width", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterDataIff,pulseWidth), },
  {PRIMITIVE, type_UINT, "burstLength", "Burst length", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct FundamentalParameterDataIff,burstLength), },
  {PRIMITIVE, type_UBYTE, "applicableModes", "Applicable modes enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FundamentalParameterDataIff,applicableModes), },
  {PRIMITIVE, type_USHORT, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct FundamentalParameterDataIff,pad2), },
  {PRIMITIVE, type_UBYTE, "pad3", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FundamentalParameterDataIff,pad3), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EightByteChunk [] = {
  {FIXED_LIST, type_BYTE, "otherParameters", "Eight bytes of arbitrary data", 8, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct EightByteChunk,otherParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_FixedDatum [] = {
  {PRIMITIVE, type_UINT, "fixedDatumID", "ID of the fixed datum", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct FixedDatum,fixedDatumID), },
  {PRIMITIVE, type_UINT, "fixedDatumValue", "Value for the fixed datum", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct FixedDatum,fixedDatumValue), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_GridAxisRecord [] = {
  {PRIMITIVE, type_USHORT, "sampleType", "type of environmental sample", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GridAxisRecord,sampleType), },
  {PRIMITIVE, type_USHORT, "dataRepresentation", "value that describes data representation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GridAxisRecord,dataRepresentation), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AggregateID [] = {
  {PRIMITIVE, type_USHORT, "site", "The site ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateID,site), },
  {PRIMITIVE, type_USHORT, "application", "The application ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateID,application), },
  {PRIMITIVE, type_USHORT, "aggregateID", "the aggregate ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateID,aggregateID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_TwoByteChunk [] = {
  {FIXED_LIST, type_BYTE, "otherParameters", "two bytes of arbitrary data", 2, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct TwoByteChunk,otherParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ClockTime [] = {
  {PRIMITIVE, type_INT, "hour", "Hours in UTC", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct ClockTime,hour), },
  {PRIMITIVE, type_UINT, "timePastHour", "Time past the hour", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ClockTime,timePastHour), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Relationship [] = {
  {PRIMITIVE, type_USHORT, "nature", "Nature of join", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct Relationship,nature), },
  {PRIMITIVE, type_USHORT, "position", "position of join", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct Relationship,position), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Vector3Float [] = {
  {PRIMITIVE, type_FLOAT, "x", "X value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Vector3Float,x), },
  {PRIMITIVE, type_FLOAT, "y", "y Value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Vector3Float,y), },
  {PRIMITIVE, type_FLOAT, "z", "Z value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Vector3Float,z), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ModulationType [] = {
  {PRIMITIVE, type_USHORT, "spreadSpectrum", "spread spectrum, 16 bit boolean array", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ModulationType,spreadSpectrum), },
  {PRIMITIVE, type_USHORT, "major", "major", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ModulationType,major), },
  {PRIMITIVE, type_USHORT, "detail", "detail", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ModulationType,detail), },
  {PRIMITIVE, type_USHORT, "system", "system", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ModulationType,system), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SimulationAddress [] = {
  {PRIMITIVE, type_USHORT, "site", "The site ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SimulationAddress,site), },
  {PRIMITIVE, type_USHORT, "application", "The application ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SimulationAddress,application), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IffFundamentalData [] = {
  {PRIMITIVE, type_UBYTE, "systemStatus", "system status", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IffFundamentalData,systemStatus), },
  {PRIMITIVE, type_UBYTE, "alternateParameter4", "Alternate parameter 4", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IffFundamentalData,alternateParameter4), },
  {PRIMITIVE, type_UBYTE, "informationLayers", "eight boolean fields", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IffFundamentalData,informationLayers), },
  {PRIMITIVE, type_UBYTE, "modifier", "enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IffFundamentalData,modifier), },
  {PRIMITIVE, type_USHORT, "parameter1", "parameter, enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IffFundamentalData,parameter1), },
  {PRIMITIVE, type_USHORT, "parameter2", "parameter, enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IffFundamentalData,parameter2), },
  {PRIMITIVE, type_USHORT, "parameter3", "parameter, enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IffFundamentalData,parameter3), },
  {PRIMITIVE, type_USHORT, "parameter4", "parameter, enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IffFundamentalData,parameter4), },
  {PRIMITIVE, type_USHORT, "parameter5", "parameter, enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IffFundamentalData,parameter5), },
  {PRIMITIVE, type_USHORT, "parameter6", "parameter, enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IffFundamentalData,parameter6), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AggregateType [] = {
  {PRIMITIVE, type_UBYTE, "aggregateKind", "Kind of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateType,aggregateKind), },
  {PRIMITIVE, type_UBYTE, "domain", "Domain of entity (air, surface, subsurface, space, etc)", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateType,domain), },
  {PRIMITIVE, type_USHORT, "country", "country to which the design of the entity is attributed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateType,country), },
  {PRIMITIVE, type_UBYTE, "category", "category of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateType,category), },
  {PRIMITIVE, type_UBYTE, "subcategory", "subcategory of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateType,subcategory), },
  {PRIMITIVE, type_UBYTE, "specific", "specific info based on subcategory field", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateType,specific), },
  {PRIMITIVE, type_UBYTE, "extra", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateType,extra), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_BeamData [] = {
  {PRIMITIVE, type_FLOAT, "beamAzimuthCenter", "Specifies the beam azimuth an elevation centers and corresponding half-angles     to describe the scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamData,beamAzimuthCenter), },
  {PRIMITIVE, type_FLOAT, "beamAzimuthSweep", "Specifies the beam azimuth sweep to determine scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamData,beamAzimuthSweep), },
  {PRIMITIVE, type_FLOAT, "beamElevationCenter", "Specifies the beam elevation center to determine scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamData,beamElevationCenter), },
  {PRIMITIVE, type_FLOAT, "beamElevationSweep", "Specifies the beam elevation sweep to determine scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamData,beamElevationSweep), },
  {PRIMITIVE, type_FLOAT, "beamSweepSync", "allows receiver to synchronize its regenerated scan pattern to     that of the emmitter. Specifies the percentage of time a scan is through its pattern from its origion.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamData,beamSweepSync), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_NamedLocation [] = {
  {PRIMITIVE, type_USHORT, "stationName", "station name enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct NamedLocation,stationName), },
  {PRIMITIVE, type_USHORT, "stationNumber", "station number", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct NamedLocation,stationNumber), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RecordSet [] = {
  {PRIMITIVE, type_UINT, "recordID", "record ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct RecordSet,recordID), },
  {PRIMITIVE, type_UINT, "recordSetSerialNumber", "record set serial number", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct RecordSet,recordSetSerialNumber), },
  {PRIMITIVE, type_USHORT, "recordLength", "record length", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RecordSet,recordLength), },
  {PRIMITIVE, type_USHORT, "recordCount", "record count", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RecordSet,recordCount), },
  {PRIMITIVE, type_USHORT, "recordValues", "^^^This is wrong--variable sized data records", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RecordSet,recordValues), },
  {PRIMITIVE, type_UBYTE, "pad4", "^^^This is wrong--variable sized padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RecordSet,pad4), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SphericalHarmonicAntennaPattern [] = {
  {PRIMITIVE, type_BYTE, "order", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct SphericalHarmonicAntennaPattern,order), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ShaftRPMs [] = {
  {PRIMITIVE, type_SHORT, "currentShaftRPMs", "Current shaft RPMs", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(short), offsetof(struct ShaftRPMs,currentShaftRPMs), },
  {PRIMITIVE, type_SHORT, "orderedShaftRPMs", "ordered shaft rpms", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(short), offsetof(struct ShaftRPMs,orderedShaftRPMs), },
  {PRIMITIVE, type_FLOAT, "shaftRPMRateOfChange", "rate of change of shaft RPMs", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct ShaftRPMs,shaftRPMRateOfChange), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IntercomCommunicationsParameters [] = {
  {PRIMITIVE, type_USHORT, "recordType", "Type of intercom parameters record", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomCommunicationsParameters,recordType), },
  {PRIMITIVE, type_USHORT, "recordLength", "length of record", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomCommunicationsParameters,recordLength), },
  {PRIMITIVE, type_UINT, "recordSpecificField", "Jerks. Looks like the committee is forcing a lookup of the record type parameter to find out how long the field is. This is a placeholder.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct IntercomCommunicationsParameters,recordSpecificField), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AcousticBeamFundamentalParameter [] = {
  {PRIMITIVE, type_USHORT, "activeEmissionParameterIndex", "parameter index", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcousticBeamFundamentalParameter,activeEmissionParameterIndex), },
  {PRIMITIVE, type_USHORT, "scanPattern", "scan pattern", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcousticBeamFundamentalParameter,scanPattern), },
  {PRIMITIVE, type_FLOAT, "beamCenterAzimuth", "beam center azimuth", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct AcousticBeamFundamentalParameter,beamCenterAzimuth), },
  {PRIMITIVE, type_FLOAT, "azimuthalBeamwidth", "azimuthal beamwidth", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct AcousticBeamFundamentalParameter,azimuthalBeamwidth), },
  {PRIMITIVE, type_FLOAT, "beamCenterDE", "beam center", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct AcousticBeamFundamentalParameter,beamCenterDE), },
  {PRIMITIVE, type_FLOAT, "deBeamwidth", "DE beamwidth (vertical beamwidth)", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct AcousticBeamFundamentalParameter,deBeamwidth), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EntityType [] = {
  {PRIMITIVE, type_UBYTE, "entityKind", "Kind of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityType,entityKind), },
  {PRIMITIVE, type_UBYTE, "domain", "Domain of entity (air, surface, subsurface, space, etc)", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityType,domain), },
  {PRIMITIVE, type_USHORT, "country", "country to which the design of the entity is attributed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EntityType,country), },
  {PRIMITIVE, type_UBYTE, "category", "category of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityType,category), },
  {PRIMITIVE, type_UBYTE, "subcategory", "subcategory of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityType,subcategory), },
  {PRIMITIVE, type_UBYTE, "specific", "specific info based on subcategory field", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityType,specific), },
  {PRIMITIVE, type_UBYTE, "extra", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityType,extra), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_FundamentalParameterData [] = {
  {PRIMITIVE, type_FLOAT, "frequency", "center frequency of the emission in hertz.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,frequency), },
  {PRIMITIVE, type_FLOAT, "frequencyRange", "Bandwidth of the frequencies corresponding to the fequency field.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,frequencyRange), },
  {PRIMITIVE, type_FLOAT, "effectiveRadiatedPower", "Effective radiated power for the emission in DdBm. For a      radar noise jammer, indicates the peak of the transmitted power.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,effectiveRadiatedPower), },
  {PRIMITIVE, type_FLOAT, "pulseRepetitionFrequency", "Average repetition frequency of the emission in hertz.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,pulseRepetitionFrequency), },
  {PRIMITIVE, type_FLOAT, "pulseWidth", "Average pulse width  of the emission in microseconds.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,pulseWidth), },
  {PRIMITIVE, type_FLOAT, "beamAzimuthCenter", "Specifies the beam azimuth an elevation centers and corresponding half-angles     to describe the scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,beamAzimuthCenter), },
  {PRIMITIVE, type_FLOAT, "beamAzimuthSweep", "Specifies the beam azimuth sweep to determine scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,beamAzimuthSweep), },
  {PRIMITIVE, type_FLOAT, "beamElevationCenter", "Specifies the beam elevation center to determine scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,beamElevationCenter), },
  {PRIMITIVE, type_FLOAT, "beamElevationSweep", "Specifies the beam elevation sweep to determine scan volume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,beamElevationSweep), },
  {PRIMITIVE, type_FLOAT, "beamSweepSync", "allows receiver to synchronize its regenerated scan pattern to     that of the emmitter. Specifies the percentage of time a scan is through its pattern from its origion.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FundamentalParameterData,beamSweepSync), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ApaData [] = {
  {PRIMITIVE, type_USHORT, "parameterIndex", "Index of APA parameter", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ApaData,parameterIndex), },
  {PRIMITIVE, type_SHORT, "parameterValue", "Index of APA parameter", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(short), offsetof(struct ApaData,parameterValue), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Environment [] = {
  {PRIMITIVE, type_UINT, "environmentType", "Record type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct Environment,environmentType), },
  {PRIMITIVE, type_UBYTE, "length", "length, in bits", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Environment,length), },
  {PRIMITIVE, type_UBYTE, "index", "Identify the sequentially numbered record index", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Environment,index), },
  {PRIMITIVE, type_UBYTE, "padding1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Environment,padding1), },
  {PRIMITIVE, type_UBYTE, "geometry", "Geometry or state record", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Environment,geometry), },
  {PRIMITIVE, type_UBYTE, "padding2", "padding to bring the total size up to a 64 bit boundry", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Environment,padding2), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AcousticEmitter [] = {
  {PRIMITIVE, type_USHORT, "acousticName", "the system for a particular UA emitter, and an enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcousticEmitter,acousticName), },
  {PRIMITIVE, type_UBYTE, "function", "The function of the acoustic system", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AcousticEmitter,function), },
  {PRIMITIVE, type_UBYTE, "acousticIdNumber", "The UA emitter identification number relative to a specific system", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AcousticEmitter,acousticIdNumber), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AngularVelocityVector [] = {
  {PRIMITIVE, type_FLOAT, "x", "velocity about the x axis", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(float), offsetof(struct AngularVelocityVector,x), },
  {PRIMITIVE, type_FLOAT, "y", "velocity about the y axis", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(float), offsetof(struct AngularVelocityVector,y), },
  {PRIMITIVE, type_FLOAT, "z", "velocity about the zaxis", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(float), offsetof(struct AngularVelocityVector,z), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AggregateMarking [] = {
  {PRIMITIVE, type_UBYTE, "characterSet", "The character set", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateMarking,characterSet), },
  {FIXED_LIST, type_BYTE, "characters", "The characters", 31, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct AggregateMarking,characters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EntityID [] = {
  {PRIMITIVE, type_USHORT, "site", "The site ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EntityID,site), },
  {PRIMITIVE, type_USHORT, "application", "The application ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EntityID,application), },
  {PRIMITIVE, type_USHORT, "entity", "the entity ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EntityID,entity), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SixByteChunk [] = {
  {FIXED_LIST, type_BYTE, "otherParameters", "six bytes of arbitrary data", 6, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct SixByteChunk,otherParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Vector3Double [] = {
  {PRIMITIVE, type_DOUBLE, "x", "X value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct Vector3Double,x), },
  {PRIMITIVE, type_DOUBLE, "y", "Y value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct Vector3Double,y), },
  {PRIMITIVE, type_DOUBLE, "z", "Z value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct Vector3Double,z), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Pdu [] = {
  {PRIMITIVE, type_UBYTE, "protocolVersion", "The version of the protocol. 5=DIS-1995, 6=DIS-1998.", 0, NULL, 0, 0, 6,0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Pdu,protocolVersion), },
  {PRIMITIVE, type_UBYTE, "exerciseID", "Exercise ID", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Pdu,exerciseID), },
  {PRIMITIVE, type_UBYTE, "pduType", "Type of pdu, unique for each PDU class", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Pdu,pduType), },
  {PRIMITIVE, type_UBYTE, "protocolFamily", "value that refers to the protocol family, eg SimulationManagement, et", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Pdu,protocolFamily), },
  {PRIMITIVE, type_UINT, "timestamp", "Timestamp value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct Pdu,timestamp), },
  {PRIMITIVE, type_USHORT, "length", "Length, in bytes, of the PDU", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct Pdu,length), },
  {PRIMITIVE, type_SHORT, "padding", "zero-filled array of padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct Pdu,padding), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_VariableDatum [] = {
  {PRIMITIVE, type_UINT, "variableDatumID", "ID of the variable datum", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct VariableDatum,variableDatumID), },
  {PRIMITIVE, type_UINT, "variableDatumLength", "length of the variable datums", 0, NULL, 0, type_EightByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct VariableDatum,variableDatumLength), },
  {VARIABLE_LIST, type_EightByteChunk, "variableDatums", "variable length list of 64-bit datums", 0, "variableDatumLength", 1, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct EightByteChunk), offsetof(struct VariableDatum,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ArticulationParameter [] = {
  {PRIMITIVE, type_UBYTE, "parameterTypeDesignator", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ArticulationParameter,parameterTypeDesignator), },
  {PRIMITIVE, type_UBYTE, "changeIndicator", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ArticulationParameter,changeIndicator), },
  {PRIMITIVE, type_USHORT, "partAttachedTo", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ArticulationParameter,partAttachedTo), },
  {PRIMITIVE, type_INT, "parameterType", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct ArticulationParameter,parameterType), },
  {PRIMITIVE, type_DOUBLE, "parameterValue", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct ArticulationParameter,parameterValue), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Marking [] = {
  {PRIMITIVE, type_UBYTE, "characterSet", "The character set", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct Marking,characterSet), },
  {FIXED_LIST, type_BYTE, "characters", "The characters", 11, NULL, 0, 0, 0, PRIMITIVE, TRUE, FALSE, sizeof(char), offsetof(struct Marking,characters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_Point [] = {
  {PRIMITIVE, type_FLOAT, "x", "x", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Point,x), },
  {PRIMITIVE, type_FLOAT, "y", "y", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct Point,y), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_PropulsionSystemData [] = {
  {PRIMITIVE, type_FLOAT, "powerSetting", "powerSetting", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct PropulsionSystemData,powerSetting), },
  {PRIMITIVE, type_FLOAT, "engineRpm", "engine RPMs", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct PropulsionSystemData,engineRpm), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EmitterSystem [] = {
  {PRIMITIVE, type_USHORT, "emitterName", "Name of the emitter, 16 bit enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EmitterSystem,emitterName), },
  {PRIMITIVE, type_UBYTE, "function", "function of the emitter, 8 bit enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EmitterSystem,function), },
  {PRIMITIVE, type_UBYTE, "emitterIdNumber", "emitter ID, 8 bit enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EmitterSystem,emitterIdNumber), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_PduContainer [] = {
  {PRIMITIVE, type_INT, "numberOfPdus", "Number of PDUs in the container list", 0, NULL, 0, type_Pdu, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct PduContainer,numberOfPdus), },
  {VARIABLE_LIST, type_Pdu, "pdus", "record sets", 0, "numberOfPdus", 0, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Pdu), offsetof(struct PduContainer,pdus), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ElectronicEmissionBeamData [] = {
  {PRIMITIVE, type_UBYTE, "beamDataLength", "This field shall specify the length of this beams data in 32 bit words", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionBeamData,beamDataLength), },
  {PRIMITIVE, type_UBYTE, "beamIDNumber", "This field shall specify a unique emitter database number assigned to differentiate between otherwise similar or identical emitter beams within an emitter system.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionBeamData,beamIDNumber), },
  {PRIMITIVE, type_USHORT, "beamParameterIndex", "This field shall specify a Beam Parameter Index number that shall be used by receiving entities in conjunction with the Emitter Name field to provide a pointer to the stored database parameters required to regenerate the beam. ", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ElectronicEmissionBeamData,beamParameterIndex), },
  {CLASSREF, type_FundamentalParameterData, "fundamentalParameterData", "Fundamental parameter data such as frequency range, beam sweep, etc.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct FundamentalParameterData), offsetof(struct ElectronicEmissionBeamData,fundamentalParameterData), },
  {PRIMITIVE, type_UBYTE, "beamFunction", "beam function of a particular beam", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionBeamData,beamFunction), },
  {PRIMITIVE, type_UBYTE, "numberOfTrackJamTargets", "Number of track/jam targets", 0, NULL, 0, type_TrackJamTarget, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionBeamData,numberOfTrackJamTargets), },
  {PRIMITIVE, type_UBYTE, "highDensityTrackJam", "wheher or not the receiving simulation apps can assume all the targets in the scan pattern are being tracked/jammed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionBeamData,highDensityTrackJam), },
  {PRIMITIVE, type_UBYTE, "pad4", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionBeamData,pad4), },
  {PRIMITIVE, type_UINT, "jammingModeSequence", "identify jamming techniques used", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ElectronicEmissionBeamData,jammingModeSequence), },
  {VARIABLE_LIST, type_TrackJamTarget, "trackJamTargets", "variable length list of track/jam targets", 0, "numberOfTrackJamTargets", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct TrackJamTarget), offsetof(struct ElectronicEmissionBeamData,trackJamTargets), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_LogisticsFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ServiceRequestPdu [] = {
  {CLASSREF, type_LogisticsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct LogisticsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "requestingEntityID", "Entity that is requesting service", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ServiceRequestPdu,requestingEntityID), },
  {CLASSREF, type_EntityID, "servicingEntityID", "Entity that is providing the service", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ServiceRequestPdu,servicingEntityID), },
  {PRIMITIVE, type_UBYTE, "serviceTypeRequested", "type of service requested", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ServiceRequestPdu,serviceTypeRequested), },
  {PRIMITIVE, type_UBYTE, "numberOfSupplyTypes", "How many requested", 0, NULL, 0, type_SupplyQuantity, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ServiceRequestPdu,numberOfSupplyTypes), },
  {PRIMITIVE, type_SHORT, "serviceRequestPadding", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct ServiceRequestPdu,serviceRequestPadding), },
  {VARIABLE_LIST, type_SupplyQuantity, "supplies", NULL, 0, "numberOfSupplyTypes", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct SupplyQuantity), offsetof(struct ServiceRequestPdu,supplies), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RepairCompletePdu [] = {
  {CLASSREF, type_LogisticsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct LogisticsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "receivingEntityID", "Entity that is receiving service", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct RepairCompletePdu,receivingEntityID), },
  {CLASSREF, type_EntityID, "repairingEntityID", "Entity that is supplying", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct RepairCompletePdu,repairingEntityID), },
  {PRIMITIVE, type_USHORT, "repair", "Enumeration for type of repair", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RepairCompletePdu,repair), },
  {PRIMITIVE, type_SHORT, "padding2", "padding, number prevents conflict with superclass ivar name", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct RepairCompletePdu,padding2), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DeadReckoningParameter [] = {
  {PRIMITIVE, type_UBYTE, "deadReckoningAlgorithm", "enumeration of what dead reckoning algorighm to use", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct DeadReckoningParameter,deadReckoningAlgorithm), },
  {FIXED_LIST, type_BYTE, "otherParameters", "other parameters to use in the dead reckoning algorithm", 15, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct DeadReckoningParameter,otherParameters), },
  {CLASSREF, type_Vector3Float, "entityLinearAcceleration", "Linear acceleration of the entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct DeadReckoningParameter,entityLinearAcceleration), },
  {CLASSREF, type_Vector3Float, "entityAngularVelocity", "angular velocity of the entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct DeadReckoningParameter,entityAngularVelocity), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_BeamAntennaPattern [] = {
  {CLASSREF, type_Orientation, "beamDirection", "The rotation that transformst he reference coordinate sytem     into the beam coordinate system. Either world coordinates or entity coordinates may be used as the     reference coordinate system, as specified by teh reference system field of the antenna pattern record.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct BeamAntennaPattern,beamDirection), },
  {PRIMITIVE, type_FLOAT, "azimuthBeamwidth", NULL, 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(float), offsetof(struct BeamAntennaPattern,azimuthBeamwidth), },
  {PRIMITIVE, type_FLOAT, "referenceSystem", NULL, 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(float), offsetof(struct BeamAntennaPattern,referenceSystem), },
  {PRIMITIVE, type_SHORT, "padding1", NULL, 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct BeamAntennaPattern,padding1), },
  {PRIMITIVE, type_BYTE, "padding2", NULL, 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(char), offsetof(struct BeamAntennaPattern,padding2), },
  {PRIMITIVE, type_FLOAT, "ez", "Magnigute of the z-component in beam coordinates at some arbitrary      single point in the mainbeam      and in the far field of the antenna.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamAntennaPattern,ez), },
  {PRIMITIVE, type_FLOAT, "ex", "Magnigute of the x-component in beam coordinates at some arbitrary      single point in the mainbeam      and in the far field of the antenna.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamAntennaPattern,ex), },
  {PRIMITIVE, type_FLOAT, "phase", "THe phase angle between Ez and Ex in radians.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct BeamAntennaPattern,phase), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SyntheticEnvironmentFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AcousticEmitterSystemData [] = {
  {PRIMITIVE, type_UBYTE, "emitterSystemDataLength", "Length of emitter system data", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AcousticEmitterSystemData,emitterSystemDataLength), },
  {PRIMITIVE, type_UBYTE, "numberOfBeams", "Number of beams", 0, NULL, 0, type_AcousticBeamData, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AcousticEmitterSystemData,numberOfBeams), },
  {PRIMITIVE, type_USHORT, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcousticEmitterSystemData,pad2), },
  {CLASSREF, type_AcousticEmitterSystem, "acousticEmitterSystem", "This field shall specify the system for a particular UA emitter.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct AcousticEmitterSystem), offsetof(struct AcousticEmitterSystemData,acousticEmitterSystem), },
  {CLASSREF, type_Vector3Float, "emitterLocation", "Represents the location wrt the entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct AcousticEmitterSystemData,emitterLocation), },
  {VARIABLE_LIST, type_AcousticBeamData, "beamRecords", "For each beam in numberOfBeams, an emitter system. This is not right--the beam records need to be at the end of the PDU, rather than attached to each system.", 0, "numberOfBeams", 1, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct AcousticBeamData), offsetof(struct AcousticEmitterSystemData,beamRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RepairResponsePdu [] = {
  {CLASSREF, type_LogisticsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct LogisticsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "receivingEntityID", "Entity that is receiving service", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct RepairResponsePdu,receivingEntityID), },
  {CLASSREF, type_EntityID, "repairingEntityID", "Entity that is supplying", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct RepairResponsePdu,repairingEntityID), },
  {PRIMITIVE, type_UBYTE, "repairResult", "Result of repair operation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RepairResponsePdu,repairResult), },
  {PRIMITIVE, type_SHORT, "padding1", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct RepairResponsePdu,padding1), },
  {PRIMITIVE, type_BYTE, "padding2", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(char), offsetof(struct RepairResponsePdu,padding2), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SimulationManagementFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {CLASSREF, type_EntityID, "originatingEntityID", "Entity that is sending message", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct SimulationManagementFamilyPdu,originatingEntityID), },
  {CLASSREF, type_EntityID, "receivingEntityID", "Entity that is intended to receive message", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct SimulationManagementFamilyPdu,receivingEntityID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AntennaLocation [] = {
  {CLASSREF, type_Vector3Double, "antennaLocation", "Location of the radiating portion of the antenna in world    coordinates", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct AntennaLocation,antennaLocation), },
  {CLASSREF, type_Vector3Float, "relativeAntennaLocation", "Location of the radiating portion of the antenna     in entity coordinates", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct AntennaLocation,relativeAntennaLocation), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DataQueryPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "ID of request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryPdu,requestID), },
  {PRIMITIVE, type_UINT, "timeInterval", "time issues between issues of Data PDUs. Zero means send once only.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryPdu,timeInterval), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Number of fixed datum records", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryPdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "Number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryPdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatums", "variable length list of fixed datums", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct DataQueryPdu,fixedDatums), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatums", "variable length list of variable length datums", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct DataQueryPdu,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_BurstDescriptor [] = {
  {CLASSREF, type_EntityType, "munition", "What munition was used in the burst", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct BurstDescriptor,munition), },
  {PRIMITIVE, type_USHORT, "warhead", "type of warhead", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct BurstDescriptor,warhead), },
  {PRIMITIVE, type_USHORT, "fuse", "type of fuse used", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct BurstDescriptor,fuse), },
  {PRIMITIVE, type_USHORT, "quantity", "how many of the munition were fired", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct BurstDescriptor,quantity), },
  {PRIMITIVE, type_USHORT, "rate", "rate at which the munition was fired", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct BurstDescriptor,rate), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_LinearObjectStatePdu [] = {
  {CLASSREF, type_SyntheticEnvironmentFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SyntheticEnvironmentFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "objectID", "Object in synthetic environment", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct LinearObjectStatePdu,objectID), },
  {CLASSREF, type_EntityID, "referencedObjectID", "Object with which this point object is associated", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct LinearObjectStatePdu,referencedObjectID), },
  {PRIMITIVE, type_USHORT, "updateNumber", "unique update number of each state transition of an object", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct LinearObjectStatePdu,updateNumber), },
  {PRIMITIVE, type_UBYTE, "forceID", "force ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct LinearObjectStatePdu,forceID), },
  {PRIMITIVE, type_UBYTE, "numberOfSegments", "number of linear segment parameters", 0, NULL, 0, type_LinearSegmentParameter, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct LinearObjectStatePdu,numberOfSegments), },
  {CLASSREF, type_SimulationAddress, "requesterID", "requesterID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SimulationAddress), offsetof(struct LinearObjectStatePdu,requesterID), },
  {CLASSREF, type_SimulationAddress, "receivingID", "receiver ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SimulationAddress), offsetof(struct LinearObjectStatePdu,receivingID), },
  {CLASSREF, type_ObjectType, "objectType", "Object type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ObjectType), offsetof(struct LinearObjectStatePdu,objectType), },
  {VARIABLE_LIST, type_LinearSegmentParameter, "linearSegmentParameters", "Linear segment parameters", 0, "numberOfSegments", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct LinearSegmentParameter), offsetof(struct LinearObjectStatePdu,linearSegmentParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_CreateEntityPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "Identifier for the request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct CreateEntityPdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RadioCommunicationsFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {CLASSREF, type_EntityID, "entityId", "ID of the entitythat is the source of the communication", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct RadioCommunicationsFamilyPdu,entityId), },
  {PRIMITIVE, type_USHORT, "radioId", "particular radio within an entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RadioCommunicationsFamilyPdu,radioId), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AcousticBeamData [] = {
  {PRIMITIVE, type_USHORT, "beamDataLength", "beam data length", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcousticBeamData,beamDataLength), },
  {PRIMITIVE, type_UBYTE, "beamIDNumber", "beamIDNumber", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AcousticBeamData,beamIDNumber), },
  {PRIMITIVE, type_USHORT, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcousticBeamData,pad2), },
  {CLASSREF, type_AcousticBeamFundamentalParameter, "fundamentalDataParameters", "fundamental data parameters", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct AcousticBeamFundamentalParameter), offsetof(struct AcousticBeamData,fundamentalDataParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IntercomSignalPdu [] = {
  {CLASSREF, type_RadioCommunicationsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct RadioCommunicationsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "entityID", "entity ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct IntercomSignalPdu,entityID), },
  {PRIMITIVE, type_USHORT, "communicationsDeviceID", "ID of communications device", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomSignalPdu,communicationsDeviceID), },
  {PRIMITIVE, type_USHORT, "encodingScheme", "encoding scheme", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomSignalPdu,encodingScheme), },
  {PRIMITIVE, type_USHORT, "tdlType", "tactical data link type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomSignalPdu,tdlType), },
  {PRIMITIVE, type_UINT, "sampleRate", "sample rate", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct IntercomSignalPdu,sampleRate), },
  {PRIMITIVE, type_USHORT, "dataLength", "data length", 0, NULL, 0, type_OneByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomSignalPdu,dataLength), },
  {PRIMITIVE, type_USHORT, "samples", "samples", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomSignalPdu,samples), },
  {VARIABLE_LIST, type_OneByteChunk, "data", "data bytes", 0, "dataLength", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct OneByteChunk), offsetof(struct IntercomSignalPdu,data), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_GridAxisRecordRepresentation2 [] = {
  {CLASSREF, type_GridAxisRecord, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct GridAxisRecord), 0 },
  {PRIMITIVE, type_USHORT, "numberOfValues", "number of values", 0, NULL, 0, type_FourByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GridAxisRecordRepresentation2,numberOfValues), },
  {VARIABLE_LIST, type_FourByteChunk, "dataValues", "variable length list of data parameters ^^^this is wrong--need padding as well", 0, "numberOfValues", 1, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FourByteChunk), offsetof(struct GridAxisRecordRepresentation2,dataValues), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_LinearSegmentParameter [] = {
  {PRIMITIVE, type_UBYTE, "segmentNumber", "number of segments", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct LinearSegmentParameter,segmentNumber), },
  {CLASSREF, type_SixByteChunk, "segmentAppearance", "segment appearance", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SixByteChunk), offsetof(struct LinearSegmentParameter,segmentAppearance), },
  {CLASSREF, type_Vector3Double, "location", "location", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct LinearSegmentParameter,location), },
  {CLASSREF, type_Orientation, "orientation", "orientation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct LinearSegmentParameter,orientation), },
  {PRIMITIVE, type_USHORT, "segmentLength", "segmentLength", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct LinearSegmentParameter,segmentLength), },
  {PRIMITIVE, type_USHORT, "segmentWidth", "segmentWidth", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct LinearSegmentParameter,segmentWidth), },
  {PRIMITIVE, type_USHORT, "segmentHeight", "segmentHeight", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct LinearSegmentParameter,segmentHeight), },
  {PRIMITIVE, type_USHORT, "segmentDepth", "segment Depth", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct LinearSegmentParameter,segmentDepth), },
  {PRIMITIVE, type_UINT, "pad1", "segment Depth", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct LinearSegmentParameter,pad1), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_GridAxisRecordRepresentation1 [] = {
  {CLASSREF, type_GridAxisRecord, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct GridAxisRecord), 0 },
  {PRIMITIVE, type_FLOAT, "fieldScale", "constant scale factor", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct GridAxisRecordRepresentation1,fieldScale), },
  {PRIMITIVE, type_FLOAT, "fieldOffset", "constant offset used to scale grid data", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct GridAxisRecordRepresentation1,fieldOffset), },
  {PRIMITIVE, type_USHORT, "numberOfValues", "Number of data values", 0, NULL, 0, type_TwoByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GridAxisRecordRepresentation1,numberOfValues), },
  {VARIABLE_LIST, type_TwoByteChunk, "dataValues", "variable length list of data parameters ^^^this is wrong--need padding as well", 0, "numberOfValues", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct TwoByteChunk), offsetof(struct GridAxisRecordRepresentation1,dataValues), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_GridAxisRecordRepresentation0 [] = {
  {CLASSREF, type_GridAxisRecord, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct GridAxisRecord), 0 },
  {PRIMITIVE, type_USHORT, "numberOfBytes", "number of bytes of environmental state data", 0, NULL, 0, type_OneByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GridAxisRecordRepresentation0,numberOfBytes), },
  {VARIABLE_LIST, type_OneByteChunk, "dataValues", "variable length list of data parameters ^^^this is wrong--need padding as well", 0, "numberOfBytes", 1, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct OneByteChunk), offsetof(struct GridAxisRecordRepresentation0,dataValues), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RemoveEntityPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "Identifier for the request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct RemoveEntityPdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ResupplyReceivedPdu [] = {
  {CLASSREF, type_LogisticsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct LogisticsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "receivingEntityID", "Entity that is receiving service", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ResupplyReceivedPdu,receivingEntityID), },
  {CLASSREF, type_EntityID, "supplyingEntityID", "Entity that is supplying", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ResupplyReceivedPdu,supplyingEntityID), },
  {PRIMITIVE, type_UBYTE, "numberOfSupplyTypes", "how many supplies are being offered", 0, NULL, 0, type_SupplyQuantity, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ResupplyReceivedPdu,numberOfSupplyTypes), },
  {PRIMITIVE, type_SHORT, "padding1", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct ResupplyReceivedPdu,padding1), },
  {PRIMITIVE, type_BYTE, "padding2", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(char), offsetof(struct ResupplyReceivedPdu,padding2), },
  {VARIABLE_LIST, type_SupplyQuantity, "supplies", NULL, 0, "numberOfSupplyTypes", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct SupplyQuantity), offsetof(struct ResupplyReceivedPdu,supplies), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_WarfareFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {CLASSREF, type_EntityID, "firingEntityID", "ID of the entity that shot", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct WarfareFamilyPdu,firingEntityID), },
  {CLASSREF, type_EntityID, "targetEntityID", "ID of the entity that is being shot at", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct WarfareFamilyPdu,targetEntityID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ElectronicEmissionSystemData [] = {
  {PRIMITIVE, type_UBYTE, "systemDataLength", "This field shall specify the length of this emitter system�s data (including beam data and its track/jam information) in 32-bit words. The length shall include the System Data Length field. ", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionSystemData,systemDataLength), },
  {PRIMITIVE, type_UBYTE, "numberOfBeams", "This field shall specify the number of beams being described in the current PDU for the system being described. ", 0, NULL, 0, type_ElectronicEmissionBeamData, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionSystemData,numberOfBeams), },
  {PRIMITIVE, type_USHORT, "emissionsPadding2", "padding.", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ElectronicEmissionSystemData,emissionsPadding2), },
  {CLASSREF, type_EmitterSystem, "emitterSystem", "This field shall specify information about a particular emitter system", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EmitterSystem), offsetof(struct ElectronicEmissionSystemData,emitterSystem), },
  {CLASSREF, type_Vector3Float, "location", "Location with respect to the entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct ElectronicEmissionSystemData,location), },
  {VARIABLE_LIST, type_ElectronicEmissionBeamData, "beamDataRecords", "variable length list of beam data records", 0, "numberOfBeams", 1, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ElectronicEmissionBeamData), offsetof(struct ElectronicEmissionSystemData,beamDataRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ActionRequestPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "Request ID that is unique", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestPdu,requestID), },
  {PRIMITIVE, type_UINT, "actionID", "identifies the action being requested", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestPdu,actionID), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Number of fixed datum records", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestPdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "Number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestPdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatums", "variable length list of fixed datums", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct ActionRequestPdu,fixedDatums), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatums", "variable length list of variable length datums", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct ActionRequestPdu,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SupplyQuantity [] = {
  {CLASSREF, type_EntityType, "supplyType", "Type of supply", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct SupplyQuantity,supplyType), },
  {PRIMITIVE, type_UBYTE, "quantity", "quantity to be supplied", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct SupplyQuantity,quantity), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AcknowledgePdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_USHORT, "acknowledgeFlag", "type of message being acknowledged", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcknowledgePdu,acknowledgeFlag), },
  {PRIMITIVE, type_USHORT, "responseFlag", "Whether or not the receiving entity was able to comply with the request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcknowledgePdu,responseFlag), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID that is unique", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct AcknowledgePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DistributedEmissionsFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IffAtcNavAidsLayer1Pdu [] = {
  {CLASSREF, type_DistributedEmissionsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct DistributedEmissionsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "emittingEntityId", "ID of the entity that is the source of the emissions", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct IffAtcNavAidsLayer1Pdu,emittingEntityId), },
  {CLASSREF, type_EventID, "eventID", "Number generated by the issuing simulation to associate realted events.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EventID), offsetof(struct IffAtcNavAidsLayer1Pdu,eventID), },
  {CLASSREF, type_Vector3Float, "location", "Location wrt entity. There is some ambugiuity in the standard here, but this is the order it is listed in the table.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct IffAtcNavAidsLayer1Pdu,location), },
  {CLASSREF, type_SystemID, "systemID", "System ID information", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SystemID), offsetof(struct IffAtcNavAidsLayer1Pdu,systemID), },
  {PRIMITIVE, type_USHORT, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IffAtcNavAidsLayer1Pdu,pad2), },
  {CLASSREF, type_IffFundamentalData, "fundamentalParameters", "fundamental parameters", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct IffFundamentalData), offsetof(struct IffAtcNavAidsLayer1Pdu,fundamentalParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SimulationManagementWithReliabilityFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {CLASSREF, type_EntityID, "originatingEntityID", "Object originatig the request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct SimulationManagementWithReliabilityFamilyPdu,originatingEntityID), },
  {CLASSREF, type_EntityID, "receivingEntityID", "Object with which this point object is associated", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct SimulationManagementWithReliabilityFamilyPdu,receivingEntityID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ActionRequestReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ActionRequestReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ActionRequestReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ActionRequestReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestReliablePdu,requestID), },
  {PRIMITIVE, type_UINT, "actionID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestReliablePdu,actionID), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Fixed datum record count", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestReliablePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "variable datum record count", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionRequestReliablePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatumRecords", "Fixed datum records", 0, "numberOfFixedDatumRecords", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct ActionRequestReliablePdu,fixedDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumRecords", "Variable datum records", 0, "numberOfVariableDatumRecords", 7, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct ActionRequestReliablePdu,variableDatumRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DesignatorPdu [] = {
  {CLASSREF, type_DistributedEmissionsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct DistributedEmissionsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "designatingEntityID", "ID of the entity designating", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct DesignatorPdu,designatingEntityID), },
  {PRIMITIVE, type_USHORT, "codeName", "This field shall specify a unique emitter database number assigned to  differentiate between otherwise similar or identical emitter beams within an emitter system.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct DesignatorPdu,codeName), },
  {CLASSREF, type_EntityID, "designatedEntityID", "ID of the entity being designated", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct DesignatorPdu,designatedEntityID), },
  {PRIMITIVE, type_USHORT, "designatorCode", "This field shall identify the designator code being used by the designating entity ", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct DesignatorPdu,designatorCode), },
  {PRIMITIVE, type_FLOAT, "designatorPower", "This field shall identify the designator output power in watts", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct DesignatorPdu,designatorPower), },
  {PRIMITIVE, type_FLOAT, "designatorWavelength", "This field shall identify the designator wavelength in units of microns", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct DesignatorPdu,designatorWavelength), },
  {CLASSREF, type_Vector3Float, "designatorSpotWrtDesignated", "designtor spot wrt the designated entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct DesignatorPdu,designatorSpotWrtDesignated), },
  {CLASSREF, type_Vector3Double, "designatorSpotLocation", "designtor spot wrt the designated entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct DesignatorPdu,designatorSpotLocation), },
  {PRIMITIVE, type_BYTE, "deadReckoningAlgorithm", "Dead reckoning algorithm", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct DesignatorPdu,deadReckoningAlgorithm), },
  {PRIMITIVE, type_USHORT, "padding1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct DesignatorPdu,padding1), },
  {PRIMITIVE, type_BYTE, "padding2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct DesignatorPdu,padding2), },
  {CLASSREF, type_Vector3Float, "entityLinearAcceleration", "linear accelleration of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct DesignatorPdu,entityLinearAcceleration), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_GriddedDataPdu [] = {
  {CLASSREF, type_SyntheticEnvironmentFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SyntheticEnvironmentFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "environmentalSimulationApplicationID", "environmental simulation application ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct GriddedDataPdu,environmentalSimulationApplicationID), },
  {PRIMITIVE, type_USHORT, "fieldNumber", "unique identifier for each piece of enviornmental data", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GriddedDataPdu,fieldNumber), },
  {PRIMITIVE, type_USHORT, "pduNumber", "sequence number for the total set of PDUS used to transmit the data", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GriddedDataPdu,pduNumber), },
  {PRIMITIVE, type_USHORT, "pduTotal", "Total number of PDUS used to transmit the data", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GriddedDataPdu,pduTotal), },
  {PRIMITIVE, type_USHORT, "coordinateSystem", "coordinate system of the grid", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GriddedDataPdu,coordinateSystem), },
  {PRIMITIVE, type_UBYTE, "numberOfGridAxes", "number of grid axes for the environmental data", 0, NULL, 0, type_GridAxisRecord, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct GriddedDataPdu,numberOfGridAxes), },
  {PRIMITIVE, type_UBYTE, "constantGrid", "are domain grid axes identidal to those of the priveious domain update?", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct GriddedDataPdu,constantGrid), },
  {CLASSREF, type_EntityType, "environmentType", "type of environment", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct GriddedDataPdu,environmentType), },
  {CLASSREF, type_Orientation, "orientation", "orientation of the data grid", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct GriddedDataPdu,orientation), },
  {PRIMITIVE, type_LONG, "sampleTime", "valid time of the enviormental data sample, 64 bit unsigned int", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(long long), offsetof(struct GriddedDataPdu,sampleTime), },
  {PRIMITIVE, type_UINT, "totalValues", "total number of all data values for all pdus for an environmental sample", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct GriddedDataPdu,totalValues), },
  {PRIMITIVE, type_UBYTE, "vectorDimension", "total number of data values at each grid point.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct GriddedDataPdu,vectorDimension), },
  {PRIMITIVE, type_USHORT, "padding1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct GriddedDataPdu,padding1), },
  {PRIMITIVE, type_UBYTE, "padding2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct GriddedDataPdu,padding2), },
  {VARIABLE_LIST, type_GridAxisRecord, "gridDataList", "Grid data ^^^This is wrong", 0, "numberOfGridAxes", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct GridAxisRecord), offsetof(struct GriddedDataPdu,gridDataList), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SetRecordReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetRecordReliablePdu,requestID), },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct SetRecordReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding. The spec is unclear and contradictory here.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SetRecordReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct SetRecordReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "numberOfRecordSets", "Number of record sets in list", 0, NULL, 0, type_RecordSet, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetRecordReliablePdu,numberOfRecordSets), },
  {VARIABLE_LIST, type_RecordSet, "recordSets", "record sets", 0, "numberOfRecordSets", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct RecordSet), offsetof(struct SetRecordReliablePdu,recordSets), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_StopFreezePdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {CLASSREF, type_ClockTime, "realWorldTime", "UTC time at which the simulation shall stop or freeze", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ClockTime), offsetof(struct StopFreezePdu,realWorldTime), },
  {PRIMITIVE, type_UBYTE, "reason", "Reason the simulation was stopped or frozen", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StopFreezePdu,reason), },
  {PRIMITIVE, type_UBYTE, "frozenBehavior", "Internal behavior of the simulation and its appearance while frozento the other participants", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StopFreezePdu,frozenBehavior), },
  {PRIMITIVE, type_SHORT, "padding1", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct StopFreezePdu,padding1), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID that is unique", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct StopFreezePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ResupplyCancelPdu [] = {
  {CLASSREF, type_LogisticsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct LogisticsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "receivingEntityID", "Entity that is receiving service", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ResupplyCancelPdu,receivingEntityID), },
  {CLASSREF, type_EntityID, "supplyingEntityID", "Entity that is supplying", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ResupplyCancelPdu,supplyingEntityID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EntityManagementFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_StartResumePdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {CLASSREF, type_ClockTime, "realWorldTime", "UTC time at which the simulation shall start or resume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ClockTime), offsetof(struct StartResumePdu,realWorldTime), },
  {CLASSREF, type_ClockTime, "simulationTime", "Simulation clock time at which the simulation shall start or resume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ClockTime), offsetof(struct StartResumePdu,simulationTime), },
  {PRIMITIVE, type_UINT, "requestID", "Identifier for the request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct StartResumePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_TransmitterPdu [] = {
  {CLASSREF, type_RadioCommunicationsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct RadioCommunicationsFamilyPdu), 0 },
  {CLASSREF, type_RadioEntityType, "radioEntityType", "linear accelleration of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct RadioEntityType), offsetof(struct TransmitterPdu,radioEntityType), },
  {PRIMITIVE, type_UBYTE, "transmitState", "transmit state", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TransmitterPdu,transmitState), },
  {PRIMITIVE, type_UBYTE, "inputSource", "input source", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TransmitterPdu,inputSource), },
  {PRIMITIVE, type_USHORT, "padding1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct TransmitterPdu,padding1), },
  {CLASSREF, type_Vector3Double, "antennaLocation", "Location of antenna", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct TransmitterPdu,antennaLocation), },
  {CLASSREF, type_Vector3Float, "relativeAntennaLocation", "relative location of antenna", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct TransmitterPdu,relativeAntennaLocation), },
  {PRIMITIVE, type_USHORT, "antennaPatternType", "antenna pattern type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct TransmitterPdu,antennaPatternType), },
  {PRIMITIVE, type_USHORT, "antennaPatternCount", "atenna pattern length", 0, NULL, 0, type_Vector3Float, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct TransmitterPdu,antennaPatternCount), },
  {PRIMITIVE, type_ULONG, "frequency", "frequency", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned long long), offsetof(struct TransmitterPdu,frequency), },
  {PRIMITIVE, type_FLOAT, "transmitFrequencyBandwidth", "transmit frequency Bandwidth", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct TransmitterPdu,transmitFrequencyBandwidth), },
  {PRIMITIVE, type_FLOAT, "power", "transmission power", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct TransmitterPdu,power), },
  {CLASSREF, type_ModulationType, "modulationType", "modulation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ModulationType), offsetof(struct TransmitterPdu,modulationType), },
  {PRIMITIVE, type_USHORT, "cryptoSystem", "crypto system enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct TransmitterPdu,cryptoSystem), },
  {PRIMITIVE, type_USHORT, "cryptoKeyId", "crypto system key identifer", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct TransmitterPdu,cryptoKeyId), },
  {PRIMITIVE, type_UBYTE, "modulationParameterCount", "how many modulation parameters we have", 0, NULL, 0, type_Vector3Float, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TransmitterPdu,modulationParameterCount), },
  {PRIMITIVE, type_USHORT, "padding2", "padding2", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct TransmitterPdu,padding2), },
  {PRIMITIVE, type_UBYTE, "padding3", "padding3", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TransmitterPdu,padding3), },
  {VARIABLE_LIST, type_Vector3Float, "modulationParametersList", "variable length list of modulation parameters", 0, "modulationParameterCount", 15, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct TransmitterPdu,modulationParametersList), },
  {VARIABLE_LIST, type_Vector3Float, "antennaPatternList", "variable length list of antenna pattern records", 0, "antennaPatternCount", 8, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct TransmitterPdu,antennaPatternList), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_TrackJamTarget [] = {
  {CLASSREF, type_EntityID, "trackJam", "track/jam target", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct TrackJamTarget,trackJam), },
  {PRIMITIVE, type_UBYTE, "emitterID", "Emitter ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TrackJamTarget,emitterID), },
  {PRIMITIVE, type_UBYTE, "beamID", "beam ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TrackJamTarget,beamID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ElectronicEmissionsPdu [] = {
  {CLASSREF, type_DistributedEmissionsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct DistributedEmissionsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "emittingEntityID", "ID of the entity emitting", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ElectronicEmissionsPdu,emittingEntityID), },
  {CLASSREF, type_EventID, "eventID", "ID of event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EventID), offsetof(struct ElectronicEmissionsPdu,eventID), },
  {PRIMITIVE, type_UBYTE, "stateUpdateIndicator", "This field shall be used to indicate if the data in the PDU represents a state update or just data that has changed since issuance of the last Electromagnetic Emission PDU [relative to the identified entity and emission system(s)].", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionsPdu,stateUpdateIndicator), },
  {PRIMITIVE, type_UBYTE, "numberOfSystems", "This field shall specify the number of emission systems being described in the current PDU.", 0, NULL, 0, type_ElectronicEmissionSystemData, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ElectronicEmissionsPdu,numberOfSystems), },
  {PRIMITIVE, type_USHORT, "paddingForEmissionsPdu", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ElectronicEmissionsPdu,paddingForEmissionsPdu), },
  {VARIABLE_LIST, type_ElectronicEmissionSystemData, "systems", "Electronic emmissions systems", 0, "numberOfSystems", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ElectronicEmissionSystemData), offsetof(struct ElectronicEmissionsPdu,systems), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ResupplyOfferPdu [] = {
  {CLASSREF, type_LogisticsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct LogisticsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "receivingEntityID", "Entity that is receiving service", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ResupplyOfferPdu,receivingEntityID), },
  {CLASSREF, type_EntityID, "supplyingEntityID", "Entity that is supplying", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ResupplyOfferPdu,supplyingEntityID), },
  {PRIMITIVE, type_UBYTE, "numberOfSupplyTypes", "how many supplies are being offered", 0, NULL, 0, type_SupplyQuantity, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ResupplyOfferPdu,numberOfSupplyTypes), },
  {PRIMITIVE, type_SHORT, "padding1", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct ResupplyOfferPdu,padding1), },
  {PRIMITIVE, type_BYTE, "padding2", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(char), offsetof(struct ResupplyOfferPdu,padding2), },
  {VARIABLE_LIST, type_SupplyQuantity, "supplies", NULL, 0, "numberOfSupplyTypes", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct SupplyQuantity), offsetof(struct ResupplyOfferPdu,supplies), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_MinefieldFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SetDataReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct SetDataReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SetDataReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct SetDataReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetDataReliablePdu,requestID), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Fixed datum record count", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetDataReliablePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "variable datum record count", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetDataReliablePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatumRecords", "Fixed datum records", 0, "numberOfFixedDatumRecords", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct SetDataReliablePdu,fixedDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumRecords", "Variable datum records", 0, "numberOfVariableDatumRecords", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct SetDataReliablePdu,variableDatumRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EventReportPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "eventType", "Type of event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct EventReportPdu,eventType), },
  {PRIMITIVE, type_UINT, "padding1", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct EventReportPdu,padding1), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Number of fixed datum records", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct EventReportPdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "Number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct EventReportPdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatums", "variable length list of fixed datums", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct EventReportPdu,fixedDatums), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatums", "variable length list of variable length datums", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct EventReportPdu,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_PointObjectStatePdu [] = {
  {CLASSREF, type_SyntheticEnvironmentFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SyntheticEnvironmentFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "objectID", "Object in synthetic environment", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct PointObjectStatePdu,objectID), },
  {CLASSREF, type_EntityID, "referencedObjectID", "Object with which this point object is associated", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct PointObjectStatePdu,referencedObjectID), },
  {PRIMITIVE, type_USHORT, "updateNumber", "unique update number of each state transition of an object", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct PointObjectStatePdu,updateNumber), },
  {PRIMITIVE, type_UBYTE, "forceID", "force ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct PointObjectStatePdu,forceID), },
  {PRIMITIVE, type_UBYTE, "modifications", "modifications", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct PointObjectStatePdu,modifications), },
  {CLASSREF, type_ObjectType, "objectType", "Object type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ObjectType), offsetof(struct PointObjectStatePdu,objectType), },
  {CLASSREF, type_Vector3Double, "objectLocation", "Object location", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct PointObjectStatePdu,objectLocation), },
  {CLASSREF, type_Orientation, "objectOrientation", "Object orientation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct PointObjectStatePdu,objectOrientation), },
  {PRIMITIVE, type_DOUBLE, "objectAppearance", "Object apperance", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct PointObjectStatePdu,objectAppearance), },
  {CLASSREF, type_SimulationAddress, "requesterID", "requesterID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SimulationAddress), offsetof(struct PointObjectStatePdu,requesterID), },
  {CLASSREF, type_SimulationAddress, "receivingID", "receiver ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SimulationAddress), offsetof(struct PointObjectStatePdu,receivingID), },
  {PRIMITIVE, type_UINT, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct PointObjectStatePdu,pad2), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EnvironmentalProcessPdu [] = {
  {CLASSREF, type_SyntheticEnvironmentFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SyntheticEnvironmentFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "environementalProcessID", "Environmental process ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct EnvironmentalProcessPdu,environementalProcessID), },
  {CLASSREF, type_EntityType, "environmentType", "Environment type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct EnvironmentalProcessPdu,environmentType), },
  {PRIMITIVE, type_UBYTE, "modelType", "model type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EnvironmentalProcessPdu,modelType), },
  {PRIMITIVE, type_UBYTE, "environmentStatus", "Environment status", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EnvironmentalProcessPdu,environmentStatus), },
  {PRIMITIVE, type_UBYTE, "numberOfEnvironmentRecords", "number of environment records ", 0, NULL, 0, type_Environment, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EnvironmentalProcessPdu,numberOfEnvironmentRecords), },
  {PRIMITIVE, type_USHORT, "sequenceNumber", "PDU sequence number for the environmentla process if pdu sequencing required", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EnvironmentalProcessPdu,sequenceNumber), },
  {VARIABLE_LIST, type_Environment, "environmentRecords", "environemt records", 0, "numberOfEnvironmentRecords", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Environment), offsetof(struct EnvironmentalProcessPdu,environmentRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DataPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "ID of request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataPdu,requestID), },
  {PRIMITIVE, type_UINT, "padding1", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataPdu,padding1), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Number of fixed datum records", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataPdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "Number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataPdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatums", "variable length list of fixed datums", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct DataPdu,fixedDatums), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatums", "variable length list of variable length datums", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct DataPdu,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IsGroupOfPdu [] = {
  {CLASSREF, type_EntityManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityManagementFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "groupEntityID", "ID of aggregated entities", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct IsGroupOfPdu,groupEntityID), },
  {PRIMITIVE, type_UBYTE, "groupedEntityCategory", "type of entities constituting the group", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IsGroupOfPdu,groupedEntityCategory), },
  {PRIMITIVE, type_UBYTE, "numberOfGroupedEntities", "Number of individual entities constituting the group", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IsGroupOfPdu,numberOfGroupedEntities), },
  {PRIMITIVE, type_UINT, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct IsGroupOfPdu,pad2), },
  {PRIMITIVE, type_DOUBLE, "latitude", "latitude", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct IsGroupOfPdu,latitude), },
  {PRIMITIVE, type_DOUBLE, "longitude", "longitude", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct IsGroupOfPdu,longitude), },
  {VARIABLE_LIST, type_VariableDatum, "groupedEntityDescriptions", "GED records about each individual entity in the group. ^^^this is wrong--need a database lookup to find the actual size of the list elements", 0, "numberOfGroupedEntities", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct IsGroupOfPdu,groupedEntityDescriptions), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_MinefieldDataPdu [] = {
  {CLASSREF, type_MinefieldFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct MinefieldFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "minefieldID", "Minefield ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct MinefieldDataPdu,minefieldID), },
  {CLASSREF, type_EntityID, "requestingEntityID", "ID of entity making request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct MinefieldDataPdu,requestingEntityID), },
  {PRIMITIVE, type_USHORT, "minefieldSequenceNumbeer", "Minefield sequence number", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct MinefieldDataPdu,minefieldSequenceNumbeer), },
  {PRIMITIVE, type_UBYTE, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldDataPdu,requestID), },
  {PRIMITIVE, type_UBYTE, "pduSequenceNumber", "pdu sequence number", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldDataPdu,pduSequenceNumber), },
  {PRIMITIVE, type_UBYTE, "numberOfPdus", "number of pdus in response", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldDataPdu,numberOfPdus), },
  {PRIMITIVE, type_UBYTE, "numberOfMinesInThisPdu", "how many mines are in this PDU", 0, NULL, 0, type_Vector3Float, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldDataPdu,numberOfMinesInThisPdu), },
  {PRIMITIVE, type_UBYTE, "numberOfSensorTypes", "how many sensor type are in this PDU", 0, NULL, 0, type_TwoByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldDataPdu,numberOfSensorTypes), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldDataPdu,pad2), },
  {PRIMITIVE, type_UINT, "dataFilter", "32 boolean fields", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct MinefieldDataPdu,dataFilter), },
  {CLASSREF, type_EntityType, "mineType", "Mine type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct MinefieldDataPdu,mineType), },
  {VARIABLE_LIST, type_TwoByteChunk, "sensorTypes", "Sensor types, each 16 bits long", 0, "numberOfSensorTypes", 8, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct TwoByteChunk), offsetof(struct MinefieldDataPdu,sensorTypes), },
  {PRIMITIVE, type_UBYTE, "pad3", "Padding to get things 32-bit aligned. ^^^this is wrong--dyanmically sized padding needed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldDataPdu,pad3), },
  {VARIABLE_LIST, type_Vector3Float, "mineLocation", "Mine locations", 0, "numberOfMinesInThisPdu", 7, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct MinefieldDataPdu,mineLocation), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_TransferControlRequestPdu [] = {
  {CLASSREF, type_EntityManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityManagementFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "orginatingEntityID", "ID of entity originating request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct TransferControlRequestPdu,orginatingEntityID), },
  {CLASSREF, type_EntityID, "recevingEntityID", "ID of entity receiving request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct TransferControlRequestPdu,recevingEntityID), },
  {PRIMITIVE, type_UINT, "requestID", "ID ofrequest", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct TransferControlRequestPdu,requestID), },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "required level of reliabliity service.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TransferControlRequestPdu,requiredReliabilityService), },
  {PRIMITIVE, type_UBYTE, "tranferType", "type of transfer desired", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TransferControlRequestPdu,tranferType), },
  {CLASSREF, type_EntityID, "transferEntityID", "The entity for which control is being requested to transfer", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct TransferControlRequestPdu,transferEntityID), },
  {PRIMITIVE, type_UBYTE, "numberOfRecordSets", "number of record sets to transfer", 0, NULL, 0, type_RecordSet, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct TransferControlRequestPdu,numberOfRecordSets), },
  {VARIABLE_LIST, type_RecordSet, "recordSets", "^^^This is wrong--the RecordSet class needs more work", 0, "numberOfRecordSets", 7, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct RecordSet), offsetof(struct TransferControlRequestPdu,recordSets), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EntityInformationFamilyPdu [] = {
  {CLASSREF, type_Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct Pdu), 0 },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AcknowledgeReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_USHORT, "acknowledgeFlag", "ack flags", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcknowledgeReliablePdu,acknowledgeFlag), },
  {PRIMITIVE, type_USHORT, "responseFlag", "response flags", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AcknowledgeReliablePdu,responseFlag), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct AcknowledgeReliablePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_StartResumeReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {CLASSREF, type_ClockTime, "realWorldTime", "time in real world for this operation to happen", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ClockTime), offsetof(struct StartResumeReliablePdu,realWorldTime), },
  {CLASSREF, type_ClockTime, "simulationTime", "time in simulation for the simulation to resume", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ClockTime), offsetof(struct StartResumeReliablePdu,simulationTime), },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StartResumeReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct StartResumeReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StartResumeReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct StartResumeReliablePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IffAtcNavAidsLayer2Pdu [] = {
  {CLASSREF, type_IffAtcNavAidsLayer1Pdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct IffAtcNavAidsLayer1Pdu), 0 },
  {CLASSREF, type_LayerHeader, "layerHeader", "layer header", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct LayerHeader), offsetof(struct IffAtcNavAidsLayer2Pdu,layerHeader), },
  {CLASSREF, type_BeamData, "beamData", "beam data", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct BeamData), offsetof(struct IffAtcNavAidsLayer2Pdu,beamData), },
  {CLASSREF, type_BeamData, "secondaryOperationalData", "Secondary operational data, 5.2.57", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct BeamData), offsetof(struct IffAtcNavAidsLayer2Pdu,secondaryOperationalData), },
  {VARIABLE_LIST, type_FundamentalParameterDataIff, "fundamentalIffParameters", "variable length list of fundamental parameters. ^^^This is wrong", 0, "pad2", 0, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FundamentalParameterDataIff), offsetof(struct IffAtcNavAidsLayer2Pdu,fundamentalIffParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ArealObjectStatePdu [] = {
  {CLASSREF, type_SyntheticEnvironmentFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SyntheticEnvironmentFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "objectID", "Object in synthetic environment", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ArealObjectStatePdu,objectID), },
  {CLASSREF, type_EntityID, "referencedObjectID", "Object with which this point object is associated", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ArealObjectStatePdu,referencedObjectID), },
  {PRIMITIVE, type_USHORT, "updateNumber", "unique update number of each state transition of an object", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ArealObjectStatePdu,updateNumber), },
  {PRIMITIVE, type_UBYTE, "forceID", "force ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ArealObjectStatePdu,forceID), },
  {PRIMITIVE, type_UBYTE, "modifications", "modifications enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct ArealObjectStatePdu,modifications), },
  {CLASSREF, type_EntityType, "objectType", "Object type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct ArealObjectStatePdu,objectType), },
  {CLASSREF, type_SixByteChunk, "objectAppearance", "Object appearance", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SixByteChunk), offsetof(struct ArealObjectStatePdu,objectAppearance), },
  {PRIMITIVE, type_USHORT, "numberOfPoints", "Number of points", 0, NULL, 0, type_Vector3Double, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ArealObjectStatePdu,numberOfPoints), },
  {CLASSREF, type_SimulationAddress, "requesterID", "requesterID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SimulationAddress), offsetof(struct ArealObjectStatePdu,requesterID), },
  {CLASSREF, type_SimulationAddress, "receivingID", "receiver ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct SimulationAddress), offsetof(struct ArealObjectStatePdu,receivingID), },
  {VARIABLE_LIST, type_Vector3Double, "objectLocation", "location of object", 0, "numberOfPoints", 8, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct ArealObjectStatePdu,objectLocation), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DataQueryReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct DataQueryReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct DataQueryReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct DataQueryReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryReliablePdu,requestID), },
  {PRIMITIVE, type_UINT, "timeInterval", "time interval between issuing data query PDUs", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryReliablePdu,timeInterval), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Fixed datum record count", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryReliablePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "variable datum record count", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataQueryReliablePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatumRecords", "Fixed datum records", 0, "numberOfFixedDatumRecords", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct DataQueryReliablePdu,fixedDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumRecords", "Variable datum records", 0, "numberOfVariableDatumRecords", 7, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct DataQueryReliablePdu,variableDatumRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_AggregateStatePdu [] = {
  {CLASSREF, type_EntityManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityManagementFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "aggregateID", "ID of aggregated entities", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct AggregateStatePdu,aggregateID), },
  {PRIMITIVE, type_UBYTE, "forceID", "force ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateStatePdu,forceID), },
  {PRIMITIVE, type_UBYTE, "aggregateState", "state of aggregate", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateStatePdu,aggregateState), },
  {CLASSREF, type_EntityType, "aggregateType", "entity type of the aggregated entities", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct AggregateStatePdu,aggregateType), },
  {PRIMITIVE, type_UINT, "formation", "formation of aggregated entities", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct AggregateStatePdu,formation), },
  {CLASSREF, type_AggregateMarking, "aggregateMarking", "marking for aggregate; first char is charset type, rest is char data", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct AggregateMarking), offsetof(struct AggregateStatePdu,aggregateMarking), },
  {CLASSREF, type_Vector3Float, "dimensions", "dimensions of bounding box for the aggregated entities, origin at the center of mass", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct AggregateStatePdu,dimensions), },
  {CLASSREF, type_Orientation, "orientation", "orientation of the bounding box", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct AggregateStatePdu,orientation), },
  {CLASSREF, type_Vector3Double, "centerOfMass", "center of mass of the aggregation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct AggregateStatePdu,centerOfMass), },
  {CLASSREF, type_Vector3Float, "velocity", "velocity of aggregation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct AggregateStatePdu,velocity), },
  {PRIMITIVE, type_USHORT, "numberOfDisAggregates", "number of aggregates", 0, NULL, 0, type_AggregateID, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateStatePdu,numberOfDisAggregates), },
  {PRIMITIVE, type_USHORT, "numberOfDisEntities", "number of entities", 0, NULL, 0, type_EntityID, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateStatePdu,numberOfDisEntities), },
  {PRIMITIVE, type_USHORT, "numberOfSilentAggregateTypes", "number of silent aggregate types", 0, NULL, 0, type_EntityType, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateStatePdu,numberOfSilentAggregateTypes), },
  {PRIMITIVE, type_USHORT, "numberOfSilentEntityTypes", "number of silent entity types", 0, NULL, 0, type_EntityType, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct AggregateStatePdu,numberOfSilentEntityTypes), },
  {VARIABLE_LIST, type_AggregateID, "aggregateIDList", "aggregates  list", 0, "numberOfDisAggregates", 11, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct AggregateID), offsetof(struct AggregateStatePdu,aggregateIDList), },
  {VARIABLE_LIST, type_EntityID, "entityIDList", "entity ID list", 0, "numberOfDisEntities", 12, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct AggregateStatePdu,entityIDList), },
  {PRIMITIVE, type_UBYTE, "pad2", "^^^padding to put the start of the next list on a 32 bit boundary. This needs to be fixed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct AggregateStatePdu,pad2), },
  {VARIABLE_LIST, type_EntityType, "silentAggregateSystemList", "silent entity types", 0, "numberOfSilentAggregateTypes", 13, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct AggregateStatePdu,silentAggregateSystemList), },
  {VARIABLE_LIST, type_EntityType, "silentEntitySystemList", "silent entity types", 0, "numberOfSilentEntityTypes", 14, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct AggregateStatePdu,silentEntitySystemList), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct AggregateStatePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumList", "variableDatums", 0, "numberOfVariableDatumRecords", 20, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct AggregateStatePdu,variableDatumList), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EntityStateUpdatePdu [] = {
  {CLASSREF, type_EntityInformationFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityInformationFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "entityID", "This field shall identify the entity issuing the PDU", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct EntityStateUpdatePdu,entityID), },
  {PRIMITIVE, type_BYTE, "padding1", "Padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct EntityStateUpdatePdu,padding1), },
  {PRIMITIVE, type_UBYTE, "numberOfArticulationParameters", "How many articulation parameters are in the variable length list", 0, NULL, 0, type_ArticulationParameter, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityStateUpdatePdu,numberOfArticulationParameters), },
  {CLASSREF, type_Vector3Float, "entityLinearVelocity", "Describes the speed of the entity in the world", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct EntityStateUpdatePdu,entityLinearVelocity), },
  {CLASSREF, type_Vector3Double, "entityLocation", "describes the location of the entity in the world", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct EntityStateUpdatePdu,entityLocation), },
  {CLASSREF, type_Orientation, "entityOrientation", "describes the orientation of the entity, in euler angles", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct EntityStateUpdatePdu,entityOrientation), },
  {PRIMITIVE, type_INT, "entityAppearance", "a series of bit flags that are used to help draw the entity, such as smoking, on fire, etc.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct EntityStateUpdatePdu,entityAppearance), },
  {VARIABLE_LIST, type_ArticulationParameter, "articulationParameters", NULL, 0, "numberOfArticulationParameters", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ArticulationParameter), offsetof(struct EntityStateUpdatePdu,articulationParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_MinefieldStatePdu [] = {
  {CLASSREF, type_MinefieldFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct MinefieldFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "minefieldID", "Minefield ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct MinefieldStatePdu,minefieldID), },
  {PRIMITIVE, type_USHORT, "minefieldSequence", "Minefield sequence", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct MinefieldStatePdu,minefieldSequence), },
  {PRIMITIVE, type_UBYTE, "forceID", "force ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldStatePdu,forceID), },
  {PRIMITIVE, type_UBYTE, "numberOfPerimeterPoints", "Number of permieter points", 0, NULL, 0, type_Point, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldStatePdu,numberOfPerimeterPoints), },
  {CLASSREF, type_EntityType, "minefieldType", "type of minefield", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct MinefieldStatePdu,minefieldType), },
  {PRIMITIVE, type_USHORT, "numberOfMineTypes", "how many mine types", 0, NULL, 0, type_EntityType, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct MinefieldStatePdu,numberOfMineTypes), },
  {CLASSREF, type_Vector3Double, "minefieldLocation", "location of minefield in world coords", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct MinefieldStatePdu,minefieldLocation), },
  {CLASSREF, type_Orientation, "minefieldOrientation", "orientation of minefield", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct MinefieldStatePdu,minefieldOrientation), },
  {PRIMITIVE, type_USHORT, "appearance", "appearance bitflags", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct MinefieldStatePdu,appearance), },
  {PRIMITIVE, type_USHORT, "protocolMode", "protocolMode", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct MinefieldStatePdu,protocolMode), },
  {VARIABLE_LIST, type_Point, "perimeterPoints", "perimeter points for the minefield", 0, "numberOfPerimeterPoints", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Point), offsetof(struct MinefieldStatePdu,perimeterPoints), },
  {VARIABLE_LIST, type_EntityType, "mineType", "Type of mines", 0, "numberOfMineTypes", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct MinefieldStatePdu,mineType), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DataReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "Request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataReliablePdu,requestID), },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct DataReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct DataReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct DataReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Fixed datum record count", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataReliablePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "variable datum record count", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct DataReliablePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatumRecords", "Fixed datum records", 0, "numberOfFixedDatumRecords", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct DataReliablePdu,fixedDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumRecords", "Variable datum records", 0, "numberOfVariableDatumRecords", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct DataReliablePdu,variableDatumRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_CommentPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Number of fixed datum records", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct CommentPdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "Number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct CommentPdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatums", "variable length list of fixed datums", 0, "numberOfFixedDatumRecords", 1, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct CommentPdu,fixedDatums), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatums", "variable length list of variable length datums", 0, "numberOfVariableDatumRecords", 2, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct CommentPdu,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_CommentReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Fixed datum record count", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct CommentReliablePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "variable datum record count", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct CommentReliablePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatumRecords", "Fixed datum records", 0, "numberOfFixedDatumRecords", 1, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct CommentReliablePdu,fixedDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumRecords", "Variable datum records", 0, "numberOfVariableDatumRecords", 2, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct CommentReliablePdu,variableDatumRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_DetonationPdu [] = {
  {CLASSREF, type_WarfareFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct WarfareFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "munitionID", "ID of muntion that was fired", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct DetonationPdu,munitionID), },
  {CLASSREF, type_EventID, "eventID", "ID firing event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EventID), offsetof(struct DetonationPdu,eventID), },
  {CLASSREF, type_Vector3Float, "velocity", "ID firing event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct DetonationPdu,velocity), },
  {CLASSREF, type_Vector3Double, "locationInWorldCoordinates", "where the detonation is, in world coordinates", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct DetonationPdu,locationInWorldCoordinates), },
  {CLASSREF, type_BurstDescriptor, "burstDescriptor", "Describes munition used", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct BurstDescriptor), offsetof(struct DetonationPdu,burstDescriptor), },
  {CLASSREF, type_Vector3Float, "locationInEntityCoordinates", "location of the detonation or impact in the target entity's coordinate system. This information should be used for damage assessment.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct DetonationPdu,locationInEntityCoordinates), },
  {PRIMITIVE, type_UBYTE, "detonationResult", "result of the explosion", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct DetonationPdu,detonationResult), },
  {PRIMITIVE, type_UBYTE, "numberOfArticulationParameters", "How many articulation parameters we have", 0, NULL, 0, type_ArticulationParameter, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct DetonationPdu,numberOfArticulationParameters), },
  {PRIMITIVE, type_SHORT, "pad", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct DetonationPdu,pad), },
  {VARIABLE_LIST, type_ArticulationParameter, "articulationParameters", NULL, 0, "numberOfArticulationParameters", 8, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ArticulationParameter), offsetof(struct DetonationPdu,articulationParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SetDataPdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "ID of request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetDataPdu,requestID), },
  {PRIMITIVE, type_UINT, "padding1", "padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetDataPdu,padding1), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Number of fixed datum records", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetDataPdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "Number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SetDataPdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatums", "variable length list of fixed datums", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct SetDataPdu,fixedDatums), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatums", "variable length list of variable length datums", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct SetDataPdu,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RecordQueryReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct RecordQueryReliablePdu,requestID), },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RecordQueryReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding. The spec is unclear and contradictory here.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RecordQueryReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RecordQueryReliablePdu,pad2), },
  {PRIMITIVE, type_USHORT, "eventType", "event type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RecordQueryReliablePdu,eventType), },
  {PRIMITIVE, type_UINT, "time", "time", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct RecordQueryReliablePdu,time), },
  {PRIMITIVE, type_UINT, "numberOfRecords", "numberOfRecords", 0, NULL, 0, type_FourByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct RecordQueryReliablePdu,numberOfRecords), },
  {VARIABLE_LIST, type_FourByteChunk, "recordIDs", "record IDs", 0, "numberOfRecords", 7, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FourByteChunk), offsetof(struct RecordQueryReliablePdu,recordIDs), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_CollisionPdu [] = {
  {CLASSREF, type_EntityInformationFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityInformationFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "issuingEntityID", "ID of the entity that issued the collision PDU", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct CollisionPdu,issuingEntityID), },
  {CLASSREF, type_EntityID, "collidingEntityID", "ID of entity that has collided with the issuing entity ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct CollisionPdu,collidingEntityID), },
  {CLASSREF, type_EventID, "eventID", "ID of event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EventID), offsetof(struct CollisionPdu,eventID), },
  {PRIMITIVE, type_UBYTE, "collisionType", "ID of event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct CollisionPdu,collisionType), },
  {PRIMITIVE, type_BYTE, "pad", "some padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(char), offsetof(struct CollisionPdu,pad), },
  {CLASSREF, type_Vector3Float, "velocity", "velocity at collision", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct CollisionPdu,velocity), },
  {PRIMITIVE, type_FLOAT, "mass", "mass of issuing entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionPdu,mass), },
  {CLASSREF, type_Vector3Float, "location", "Location with respect to entity the issuing entity collided with", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct CollisionPdu,location), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ActionResponsePdu [] = {
  {CLASSREF, type_SimulationManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "Request ID that is unique", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponsePdu,requestID), },
  {PRIMITIVE, type_UINT, "requestStatus", "Status of response", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponsePdu,requestStatus), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Number of fixed datum records", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponsePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "Number of variable datum records", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponsePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatums", "variable length list of fixed datums", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct ActionResponsePdu,fixedDatums), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatums", "variable length list of variable length datums", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct ActionResponsePdu,variableDatums), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_FirePdu [] = {
  {CLASSREF, type_WarfareFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct WarfareFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "munitionID", "ID of the munition that is being shot", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct FirePdu,munitionID), },
  {CLASSREF, type_EventID, "eventID", "ID of event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EventID), offsetof(struct FirePdu,eventID), },
  {PRIMITIVE, type_INT, "fireMissionIndex", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct FirePdu,fireMissionIndex), },
  {CLASSREF, type_Vector3Double, "locationInWorldCoordinates", "location of the firing event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct FirePdu,locationInWorldCoordinates), },
  {CLASSREF, type_BurstDescriptor, "burstDescriptor", "Describes munitions used in the firing event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct BurstDescriptor), offsetof(struct FirePdu,burstDescriptor), },
  {CLASSREF, type_Vector3Float, "velocity", "Velocity of the ammunition", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct FirePdu,velocity), },
  {PRIMITIVE, type_FLOAT, "range", "range to the target", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FirePdu,range), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ReceiverPdu [] = {
  {CLASSREF, type_RadioCommunicationsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct RadioCommunicationsFamilyPdu), 0 },
  {PRIMITIVE, type_USHORT, "receiverState", "encoding scheme used, and enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ReceiverPdu,receiverState), },
  {PRIMITIVE, type_USHORT, "padding1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ReceiverPdu,padding1), },
  {PRIMITIVE, type_FLOAT, "receivedPoser", "received power", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct ReceiverPdu,receivedPoser), },
  {CLASSREF, type_EntityID, "transmitterEntityId", "ID of transmitter", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct ReceiverPdu,transmitterEntityId), },
  {PRIMITIVE, type_USHORT, "transmitterRadioId", "ID of transmitting radio", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct ReceiverPdu,transmitterRadioId), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_UaPdu [] = {
  {CLASSREF, type_DistributedEmissionsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct DistributedEmissionsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "emittingEntityID", "ID of the entity that is the source of the emission", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct UaPdu,emittingEntityID), },
  {CLASSREF, type_EventID, "eventID", "ID of event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EventID), offsetof(struct UaPdu,eventID), },
  {PRIMITIVE, type_BYTE, "stateChangeIndicator", "This field shall be used to indicate whether the data in the UA PDU represent a state update or data that have changed since issuance of the last UA PDU", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct UaPdu,stateChangeIndicator), },
  {PRIMITIVE, type_BYTE, "pad", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct UaPdu,pad), },
  {PRIMITIVE, type_USHORT, "passiveParameterIndex", "This field indicates which database record (or file) shall be used in the definition of passive signature (unintentional) emissions of the entity. The indicated database record (or  file) shall define all noise generated as a function of propulsion plant configurations and associated  auxiliaries.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct UaPdu,passiveParameterIndex), },
  {PRIMITIVE, type_UBYTE, "propulsionPlantConfiguration", "This field shall specify the entity propulsion plant configuration. This field is used to determine the passive signature characteristics of an entity.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct UaPdu,propulsionPlantConfiguration), },
  {PRIMITIVE, type_UBYTE, "numberOfShafts", " This field shall represent the number of shafts on a platform", 0, NULL, 0, type_ShaftRPMs, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct UaPdu,numberOfShafts), },
  {PRIMITIVE, type_UBYTE, "numberOfAPAs", "This field shall indicate the number of APAs described in the current UA PDU", 0, NULL, 0, type_ApaData, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct UaPdu,numberOfAPAs), },
  {PRIMITIVE, type_UBYTE, "numberOfUAEmitterSystems", "This field shall specify the number of UA emitter systems being described in the current UA PDU", 0, NULL, 0, type_AcousticEmitterSystemData, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct UaPdu,numberOfUAEmitterSystems), },
  {VARIABLE_LIST, type_ShaftRPMs, "shaftRPMs", "shaft RPM values", 0, "numberOfShafts", 7, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ShaftRPMs), offsetof(struct UaPdu,shaftRPMs), },
  {VARIABLE_LIST, type_ApaData, "apaData", "apaData", 0, "numberOfAPAs", 8, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ApaData), offsetof(struct UaPdu,apaData), },
  {VARIABLE_LIST, type_AcousticEmitterSystemData, "emitterSystems", NULL, 0, "numberOfUAEmitterSystems", 9, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct AcousticEmitterSystemData), offsetof(struct UaPdu,emitterSystems), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IntercomControlPdu [] = {
  {CLASSREF, type_RadioCommunicationsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct RadioCommunicationsFamilyPdu), 0 },
  {PRIMITIVE, type_UBYTE, "controlType", "control type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IntercomControlPdu,controlType), },
  {PRIMITIVE, type_UBYTE, "communicationsChannelType", "control type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IntercomControlPdu,communicationsChannelType), },
  {CLASSREF, type_EntityID, "sourceEntityID", "Source entity ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct IntercomControlPdu,sourceEntityID), },
  {PRIMITIVE, type_UBYTE, "sourceCommunicationsDeviceID", "The specific intercom device being simulated within an entity.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IntercomControlPdu,sourceCommunicationsDeviceID), },
  {PRIMITIVE, type_UBYTE, "sourceLineID", "Line number to which the intercom control refers", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IntercomControlPdu,sourceLineID), },
  {PRIMITIVE, type_UBYTE, "transmitPriority", "priority of this message relative to transmissons from other intercom devices", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IntercomControlPdu,transmitPriority), },
  {PRIMITIVE, type_UBYTE, "transmitLineState", "current transmit state of the line", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IntercomControlPdu,transmitLineState), },
  {PRIMITIVE, type_UBYTE, "command", "detailed type requested.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct IntercomControlPdu,command), },
  {CLASSREF, type_EntityID, "masterEntityID", "eid of the entity that has created this intercom channel.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct IntercomControlPdu,masterEntityID), },
  {PRIMITIVE, type_USHORT, "masterCommunicationsDeviceID", "specific intercom device that has created this intercom channel", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct IntercomControlPdu,masterCommunicationsDeviceID), },
  {PRIMITIVE, type_UINT, "intercomParametersLength", "number of intercom parameters", 0, NULL, 0, type_IntercomCommunicationsParameters, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct IntercomControlPdu,intercomParametersLength), },
  {VARIABLE_LIST, type_IntercomCommunicationsParameters, "intercomParameters", "^^^This is wrong--the length of the data field is variable. Using a long for now.", 0, "intercomParametersLength", 11, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct IntercomCommunicationsParameters), offsetof(struct IntercomControlPdu,intercomParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SignalPdu [] = {
  {CLASSREF, type_RadioCommunicationsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct RadioCommunicationsFamilyPdu), 0 },
  {PRIMITIVE, type_USHORT, "encodingScheme", "encoding scheme used, and enumeration", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SignalPdu,encodingScheme), },
  {PRIMITIVE, type_USHORT, "tdlType", "tdl type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SignalPdu,tdlType), },
  {PRIMITIVE, type_UINT, "sampleRate", "sample rate", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct SignalPdu,sampleRate), },
  {PRIMITIVE, type_SHORT, "dataLength", "length od data", 0, NULL, 0, type_OneByteChunk, 0, 0, FALSE, FALSE, sizeof(short), offsetof(struct SignalPdu,dataLength), },
  {PRIMITIVE, type_SHORT, "samples", "number of samples", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(short), offsetof(struct SignalPdu,samples), },
  {VARIABLE_LIST, type_OneByteChunk, "data", "list of eight bit values", 0, "dataLength", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct OneByteChunk), offsetof(struct SignalPdu,data), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_RemoveEntityReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RemoveEntityReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct RemoveEntityReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct RemoveEntityReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct RemoveEntityReliablePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_SeesPdu [] = {
  {CLASSREF, type_DistributedEmissionsFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct DistributedEmissionsFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "orginatingEntityID", "Originating entity ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct SeesPdu,orginatingEntityID), },
  {PRIMITIVE, type_USHORT, "infraredSignatureRepresentationIndex", "IR Signature representation index", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SeesPdu,infraredSignatureRepresentationIndex), },
  {PRIMITIVE, type_USHORT, "acousticSignatureRepresentationIndex", "acoustic Signature representation index", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SeesPdu,acousticSignatureRepresentationIndex), },
  {PRIMITIVE, type_USHORT, "radarCrossSectionSignatureRepresentationIndex", "radar cross section representation index", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SeesPdu,radarCrossSectionSignatureRepresentationIndex), },
  {PRIMITIVE, type_USHORT, "numberOfPropulsionSystems", "how many propulsion systems", 0, NULL, 0, type_PropulsionSystemData, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SeesPdu,numberOfPropulsionSystems), },
  {PRIMITIVE, type_USHORT, "numberOfVectoringNozzleSystems", "how many vectoring nozzle systems", 0, NULL, 0, type_VectoringNozzleSystemData, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct SeesPdu,numberOfVectoringNozzleSystems), },
  {VARIABLE_LIST, type_PropulsionSystemData, "propulsionSystemData", "variable length list of propulsion system data", 0, "numberOfPropulsionSystems", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct PropulsionSystemData), offsetof(struct SeesPdu,propulsionSystemData), },
  {VARIABLE_LIST, type_VectoringNozzleSystemData, "vectoringSystemData", "variable length list of vectoring system data", 0, "numberOfVectoringNozzleSystems", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VectoringNozzleSystemData), offsetof(struct SeesPdu,vectoringSystemData), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_CreateEntityReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UBYTE, "requiredReliabilityService", "level of reliability service used for this transaction", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct CreateEntityReliablePdu,requiredReliabilityService), },
  {PRIMITIVE, type_USHORT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct CreateEntityReliablePdu,pad1), },
  {PRIMITIVE, type_UBYTE, "pad2", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct CreateEntityReliablePdu,pad2), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct CreateEntityReliablePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_StopFreezeReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {CLASSREF, type_ClockTime, "realWorldTime", "time in real world for this operation to happen", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct ClockTime), offsetof(struct StopFreezeReliablePdu,realWorldTime), },
  {PRIMITIVE, type_UBYTE, "reason", "Reason for stopping/freezing simulation", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StopFreezeReliablePdu,reason), },
  {PRIMITIVE, type_UBYTE, "frozenBehavior", "internal behvior of the simulation while frozen", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StopFreezeReliablePdu,frozenBehavior), },
  {PRIMITIVE, type_UBYTE, "requiredReliablityService", "reliablity level", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StopFreezeReliablePdu,requiredReliablityService), },
  {PRIMITIVE, type_UBYTE, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct StopFreezeReliablePdu,pad1), },
  {PRIMITIVE, type_UINT, "requestID", "Request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct StopFreezeReliablePdu,requestID), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EventReportReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_USHORT, "eventType", "Event type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct EventReportReliablePdu,eventType), },
  {PRIMITIVE, type_UINT, "pad1", "padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct EventReportReliablePdu,pad1), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Fixed datum record count", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct EventReportReliablePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "variable datum record count", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct EventReportReliablePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatumRecords", "Fixed datum records", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct EventReportReliablePdu,fixedDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumRecords", "Variable datum records", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct EventReportReliablePdu,variableDatumRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_MinefieldResponseNackPdu [] = {
  {CLASSREF, type_MinefieldFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct MinefieldFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "minefieldID", "Minefield ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct MinefieldResponseNackPdu,minefieldID), },
  {CLASSREF, type_EntityID, "requestingEntityID", "entity ID making the request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct MinefieldResponseNackPdu,requestingEntityID), },
  {PRIMITIVE, type_UBYTE, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldResponseNackPdu,requestID), },
  {PRIMITIVE, type_UBYTE, "numberOfMissingPdus", "how many pdus were missing", 0, NULL, 0, type_EightByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldResponseNackPdu,numberOfMissingPdus), },
  {VARIABLE_LIST, type_EightByteChunk, "missingPduSequenceNumbers", "PDU sequence numbers that were missing", 0, "numberOfMissingPdus", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct EightByteChunk), offsetof(struct MinefieldResponseNackPdu,missingPduSequenceNumbers), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_CollisionElasticPdu [] = {
  {CLASSREF, type_EntityInformationFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityInformationFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "issuingEntityID", "ID of the entity that issued the collision PDU", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct CollisionElasticPdu,issuingEntityID), },
  {CLASSREF, type_EntityID, "collidingEntityID", "ID of entity that has collided with the issuing entity ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct CollisionElasticPdu,collidingEntityID), },
  {CLASSREF, type_EventID, "collisionEventID", "ID of event", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EventID), offsetof(struct CollisionElasticPdu,collisionEventID), },
  {PRIMITIVE, type_SHORT, "pad", "some padding", 0, NULL, 0, 0, 0,0, FALSE, FALSE, sizeof(short), offsetof(struct CollisionElasticPdu,pad), },
  {CLASSREF, type_Vector3Float, "contactVelocity", "velocity at collision", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct CollisionElasticPdu,contactVelocity), },
  {PRIMITIVE, type_FLOAT, "mass", "mass of issuing entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,mass), },
  {CLASSREF, type_Vector3Float, "location", "Location with respect to entity the issuing entity collided with", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct CollisionElasticPdu,location), },
  {PRIMITIVE, type_FLOAT, "collisionResultXX", "tensor values", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,collisionResultXX), },
  {PRIMITIVE, type_FLOAT, "collisionResultXY", "tensor values", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,collisionResultXY), },
  {PRIMITIVE, type_FLOAT, "collisionResultXZ", "tensor values", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,collisionResultXZ), },
  {PRIMITIVE, type_FLOAT, "collisionResultYY", "tensor values", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,collisionResultYY), },
  {PRIMITIVE, type_FLOAT, "collisionResultYZ", "tensor values", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,collisionResultYZ), },
  {PRIMITIVE, type_FLOAT, "collisionResultZZ", "tensor values", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,collisionResultZZ), },
  {CLASSREF, type_Vector3Float, "unitSurfaceNormal", "This record shall represent the normal vector to the surface at the point of collision detection. The surface normal shall be represented in world coordinates.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct CollisionElasticPdu,unitSurfaceNormal), },
  {PRIMITIVE, type_FLOAT, "coefficientOfRestitution", "This field shall represent the degree to which energy is conserved in a collision", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct CollisionElasticPdu,coefficientOfRestitution), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_ActionResponseReliablePdu [] = {
  {CLASSREF, type_SimulationManagementWithReliabilityFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct SimulationManagementWithReliabilityFamilyPdu), 0 },
  {PRIMITIVE, type_UINT, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponseReliablePdu,requestID), },
  {PRIMITIVE, type_UINT, "responseStatus", "status of response", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponseReliablePdu,responseStatus), },
  {PRIMITIVE, type_UINT, "numberOfFixedDatumRecords", "Fixed datum record count", 0, NULL, 0, type_FixedDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponseReliablePdu,numberOfFixedDatumRecords), },
  {PRIMITIVE, type_UINT, "numberOfVariableDatumRecords", "variable datum record count", 0, NULL, 0, type_VariableDatum, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct ActionResponseReliablePdu,numberOfVariableDatumRecords), },
  {VARIABLE_LIST, type_FixedDatum, "fixedDatumRecords", "Fixed datum records", 0, "numberOfFixedDatumRecords", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct FixedDatum), offsetof(struct ActionResponseReliablePdu,fixedDatumRecords), },
  {VARIABLE_LIST, type_VariableDatum, "variableDatumRecords", "Variable datum records", 0, "numberOfVariableDatumRecords", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct VariableDatum), offsetof(struct ActionResponseReliablePdu,variableDatumRecords), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_IsPartOfPdu [] = {
  {CLASSREF, type_EntityManagementFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityManagementFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "orginatingEntityID", "ID of entity originating PDU", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct IsPartOfPdu,orginatingEntityID), },
  {CLASSREF, type_EntityID, "receivingEntityID", "ID of entity receiving PDU", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct IsPartOfPdu,receivingEntityID), },
  {CLASSREF, type_Relationship, "relationship", "relationship of joined parts", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Relationship), offsetof(struct IsPartOfPdu,relationship), },
  {CLASSREF, type_Vector3Float, "partLocation", "location of part; centroid of part in host's coordinate system. x=range, y=bearing, z=0", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct IsPartOfPdu,partLocation), },
  {CLASSREF, type_NamedLocation, "namedLocationID", "named location", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct NamedLocation), offsetof(struct IsPartOfPdu,namedLocationID), },
  {CLASSREF, type_EntityType, "partEntityType", "entity type", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct IsPartOfPdu,partEntityType), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_MinefieldQueryPdu [] = {
  {CLASSREF, type_MinefieldFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct MinefieldFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "minefieldID", "Minefield ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct MinefieldQueryPdu,minefieldID), },
  {CLASSREF, type_EntityID, "requestingEntityID", "EID of entity making the request", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct MinefieldQueryPdu,requestingEntityID), },
  {PRIMITIVE, type_UBYTE, "requestID", "request ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldQueryPdu,requestID), },
  {PRIMITIVE, type_UBYTE, "numberOfPerimeterPoints", "Number of perimeter points for the minefield", 0, NULL, 0, type_Point, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldQueryPdu,numberOfPerimeterPoints), },
  {PRIMITIVE, type_UBYTE, "pad2", "Padding", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldQueryPdu,pad2), },
  {PRIMITIVE, type_UBYTE, "numberOfSensorTypes", "Number of sensor types", 0, NULL, 0, type_TwoByteChunk, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct MinefieldQueryPdu,numberOfSensorTypes), },
  {PRIMITIVE, type_UINT, "dataFilter", "data filter, 32 boolean fields", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned int), offsetof(struct MinefieldQueryPdu,dataFilter), },
  {CLASSREF, type_EntityType, "requestedMineType", "Entity type of mine being requested", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct MinefieldQueryPdu,requestedMineType), },
  {VARIABLE_LIST, type_Point, "requestedPerimeterPoints", "perimeter points of request", 0, "numberOfPerimeterPoints", 4, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct Point), offsetof(struct MinefieldQueryPdu,requestedPerimeterPoints), },
  {VARIABLE_LIST, type_TwoByteChunk, "sensorTypes", "Sensor types, each 16 bits long", 0, "numberOfSensorTypes", 6, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct TwoByteChunk), offsetof(struct MinefieldQueryPdu,sensorTypes), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_EntityStatePdu [] = {
  {CLASSREF, type_EntityInformationFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityInformationFamilyPdu), 0 },
  {CLASSREF, type_EntityID, "entityID", "Unique ID for an entity that is tied to this state information", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityID), offsetof(struct EntityStatePdu,entityID), },
  {PRIMITIVE, type_UBYTE, "forceId", "What force this entity is affiliated with, eg red, blue, neutral, etc", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct EntityStatePdu,forceId), },
  {PRIMITIVE, type_BYTE, "numberOfArticulationParameters", "How many articulation parameters are in the variable length list", 0, NULL, 0, type_ArticulationParameter, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct EntityStatePdu,numberOfArticulationParameters), },
  {CLASSREF, type_EntityType, "entityType", "Describes the type of entity in the world", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct EntityStatePdu,entityType), },
  {CLASSREF, type_EntityType, "alternativeEntityType", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct EntityType), offsetof(struct EntityStatePdu,alternativeEntityType), },
  {CLASSREF, type_Vector3Float, "entityLinearVelocity", "Describes the speed of the entity in the world", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Float), offsetof(struct EntityStatePdu,entityLinearVelocity), },
  {CLASSREF, type_Vector3Double, "entityLocation", "describes the location of the entity in the world", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Vector3Double), offsetof(struct EntityStatePdu,entityLocation), },
  {CLASSREF, type_Orientation, "entityOrientation", "describes the orientation of the entity, in euler angles", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Orientation), offsetof(struct EntityStatePdu,entityOrientation), },
  {PRIMITIVE, type_INT, "entityAppearance", "a series of bit flags that are used to help draw the entity, such as smoking, on fire, etc.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct EntityStatePdu,entityAppearance), },
  {CLASSREF, type_DeadReckoningParameter, "deadReckoningParameters", "parameters used for dead reckoning", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct DeadReckoningParameter), offsetof(struct EntityStatePdu,deadReckoningParameters), },
  {CLASSREF, type_Marking, "marking", "characters that can be used for debugging, or to draw unique strings on the side of entities in the world", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(struct Marking), offsetof(struct EntityStatePdu,marking), },
  {PRIMITIVE, type_INT, "capabilities", "a series of bit flags", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct EntityStatePdu,capabilities), },
  {VARIABLE_LIST, type_ArticulationParameter, "articulationParameters", "variable length list of articulation parameters", 0, "numberOfArticulationParameters", 3, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ArticulationParameter), offsetof(struct EntityStatePdu,articulationParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
struct disfieldattr FIELDS_FastEntityStatePdu [] = {
  {CLASSREF, type_EntityInformationFamilyPdu, "super", NULL,0,NULL, 0,0,0,0,0,UNSET,(int)sizeof(struct EntityInformationFamilyPdu), 0 },
  {PRIMITIVE, type_USHORT, "site", "The site ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct FastEntityStatePdu,site), },
  {PRIMITIVE, type_USHORT, "application", "The application ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct FastEntityStatePdu,application), },
  {PRIMITIVE, type_USHORT, "entity", "the entity ID", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct FastEntityStatePdu,entity), },
  {PRIMITIVE, type_UBYTE, "forceId", "what force this entity is affiliated with, eg red, blue, neutral, etc", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,forceId), },
  {PRIMITIVE, type_BYTE, "numberOfArticulationParameters", "How many articulation parameters are in the variable length list", 0, NULL, 0, type_ArticulationParameter, 0, 0, FALSE, FALSE, sizeof(char), offsetof(struct FastEntityStatePdu,numberOfArticulationParameters), },
  {PRIMITIVE, type_UBYTE, "entityKind", "Kind of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,entityKind), },
  {PRIMITIVE, type_UBYTE, "domain", "Domain of entity (air, surface, subsurface, space, etc)", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,domain), },
  {PRIMITIVE, type_USHORT, "country", "country to which the design of the entity is attributed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct FastEntityStatePdu,country), },
  {PRIMITIVE, type_UBYTE, "category", "category of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,category), },
  {PRIMITIVE, type_UBYTE, "subcategory", "subcategory of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,subcategory), },
  {PRIMITIVE, type_UBYTE, "specific", "specific info based on subcategory field", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,specific), },
  {PRIMITIVE, type_UBYTE, "extra", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,extra), },
  {PRIMITIVE, type_UBYTE, "altEntityKind", "Kind of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,altEntityKind), },
  {PRIMITIVE, type_UBYTE, "altDomain", "Domain of entity (air, surface, subsurface, space, etc)", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,altDomain), },
  {PRIMITIVE, type_USHORT, "altCountry", "country to which the design of the entity is attributed", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned short), offsetof(struct FastEntityStatePdu,altCountry), },
  {PRIMITIVE, type_UBYTE, "altCategory", "category of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,altCategory), },
  {PRIMITIVE, type_UBYTE, "altSubcategory", "subcategory of entity", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,altSubcategory), },
  {PRIMITIVE, type_UBYTE, "altSpecific", "specific info based on subcategory field", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,altSpecific), },
  {PRIMITIVE, type_UBYTE, "altExtra", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,altExtra), },
  {PRIMITIVE, type_FLOAT, "xVelocity", "X velo", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,xVelocity), },
  {PRIMITIVE, type_FLOAT, "yVelocity", "y Value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,yVelocity), },
  {PRIMITIVE, type_FLOAT, "zVelocity", "Z value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,zVelocity), },
  {PRIMITIVE, type_DOUBLE, "xLocation", "X value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct FastEntityStatePdu,xLocation), },
  {PRIMITIVE, type_DOUBLE, "yLocation", "y Value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct FastEntityStatePdu,yLocation), },
  {PRIMITIVE, type_DOUBLE, "zLocation", "Z value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(double), offsetof(struct FastEntityStatePdu,zLocation), },
  {PRIMITIVE, type_FLOAT, "psi", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,psi), },
  {PRIMITIVE, type_FLOAT, "theta", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,theta), },
  {PRIMITIVE, type_FLOAT, "phi", NULL, 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,phi), },
  {PRIMITIVE, type_INT, "entityAppearance", "a series of bit flags that are used to help draw the entity, such as smoking, on fire, etc.", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct FastEntityStatePdu,entityAppearance), },
  {PRIMITIVE, type_UBYTE, "deadReckoningAlgorithm", "enumeration of what dead reckoning algorighm to use", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(unsigned char), offsetof(struct FastEntityStatePdu,deadReckoningAlgorithm), },
  {FIXED_LIST, type_BYTE, "otherParameters", "other parameters to use in the dead reckoning algorithm", 15, NULL, 0, 0, 0, PRIMITIVE, FALSE, FALSE, sizeof(char), offsetof(struct FastEntityStatePdu,otherParameters), },
  {PRIMITIVE, type_FLOAT, "xAcceleration", "X value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,xAcceleration), },
  {PRIMITIVE, type_FLOAT, "yAcceleration", "y Value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,yAcceleration), },
  {PRIMITIVE, type_FLOAT, "zAcceleration", "Z value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,zAcceleration), },
  {PRIMITIVE, type_FLOAT, "xAngularVelocity", "X value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,xAngularVelocity), },
  {PRIMITIVE, type_FLOAT, "yAngularVelocity", "y Value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,yAngularVelocity), },
  {PRIMITIVE, type_FLOAT, "zAngularVelocity", "Z value", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(float), offsetof(struct FastEntityStatePdu,zAngularVelocity), },
  {FIXED_LIST, type_BYTE, "marking", "characters that can be used for debugging, or to draw unique strings on the side of entities in the world", 12, NULL, 0, 0, 0, PRIMITIVE, TRUE, FALSE, sizeof(char), offsetof(struct FastEntityStatePdu,marking), },
  {PRIMITIVE, type_INT, "capabilities", "a series of bit flags", 0, NULL, 0, 0, 0, 0, FALSE, FALSE, sizeof(int), offsetof(struct FastEntityStatePdu,capabilities), },
  {VARIABLE_LIST, type_ArticulationParameter, "articulationParameters", "variable length list of articulation parameters", 0, "numberOfArticulationParameters", 5, 0, 0, CLASSREF, FALSE, FALSE, sizeof(struct ArticulationParameter), offsetof(struct FastEntityStatePdu,articulationParameters), },
  {-1,0,NULL,NULL,0,NULL,0,0,0,0,0,0,0,0},
};
void ctor_pduHeader(void *pdutype, unsigned char pduType, unsigned char protocolFamily){
	struct Pdu *header = (struct Pdu*)pdutype;
	header->pduType = pduType;
	header->protocolFamily = protocolFamily;
}
struct dis_class { 
	struct disfieldattr *fields;
	unsigned char pduType;
	unsigned char protocolFamily;
};
// class table - same order as type_ enum
struct dis_class DIS_CLASS [] = {
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {NULL,0,0},
  {FIELDS_SystemID,0,0},
  {FIELDS_RadioEntityType,0,0},
  {FIELDS_LayerHeader,0,0},
  {FIELDS_AcousticEmitterSystem,0,0},
  {FIELDS_FourByteChunk,0,0},
  {FIELDS_Orientation,0,0},
  {FIELDS_OneByteChunk,0,0},
  {FIELDS_EventID,0,0},
  {FIELDS_VectoringNozzleSystemData,0,0},
  {FIELDS_ObjectType,0,0},
  {FIELDS_FundamentalParameterDataIff,0,0},
  {FIELDS_EightByteChunk,0,0},
  {FIELDS_FixedDatum,0,0},
  {FIELDS_GridAxisRecord,0,0},
  {FIELDS_AggregateID,0,0},
  {FIELDS_TwoByteChunk,0,0},
  {FIELDS_ClockTime,0,0},
  {FIELDS_Relationship,0,0},
  {FIELDS_Vector3Float,0,0},
  {FIELDS_ModulationType,0,0},
  {FIELDS_SimulationAddress,0,0},
  {FIELDS_IffFundamentalData,0,0},
  {FIELDS_AggregateType,0,0},
  {FIELDS_BeamData,0,0},
  {FIELDS_NamedLocation,0,0},
  {FIELDS_RecordSet,0,0},
  {FIELDS_SphericalHarmonicAntennaPattern,0,0},
  {FIELDS_ShaftRPMs,0,0},
  {FIELDS_IntercomCommunicationsParameters,0,0},
  {FIELDS_AcousticBeamFundamentalParameter,0,0},
  {FIELDS_EntityType,0,0},
  {FIELDS_FundamentalParameterData,0,0},
  {FIELDS_ApaData,0,0},
  {FIELDS_Environment,0,0},
  {FIELDS_AcousticEmitter,0,0},
  {FIELDS_AngularVelocityVector,0,0},
  {FIELDS_AggregateMarking,0,0},
  {FIELDS_EntityID,0,0},
  {FIELDS_SixByteChunk,0,0},
  {FIELDS_Vector3Double,0,0},
  {FIELDS_Pdu,0,0},
  {FIELDS_VariableDatum,0,0},
  {FIELDS_ArticulationParameter,0,0},
  {FIELDS_Marking,0,0},
  {FIELDS_Point,0,0},
  {FIELDS_PropulsionSystemData,0,0},
  {FIELDS_EmitterSystem,0,0},
  {FIELDS_PduContainer,0,0},
  {FIELDS_ElectronicEmissionBeamData,0,0},
  {FIELDS_LogisticsFamilyPdu,0,3},
  {FIELDS_ServiceRequestPdu,5,0},
  {FIELDS_RepairCompletePdu,9,0},
  {FIELDS_DeadReckoningParameter,0,0},
  {FIELDS_BeamAntennaPattern,0,0},
  {FIELDS_SyntheticEnvironmentFamilyPdu,0,9},
  {FIELDS_AcousticEmitterSystemData,0,0},
  {FIELDS_RepairResponsePdu,10,0},
  {FIELDS_SimulationManagementFamilyPdu,0,5},
  {FIELDS_AntennaLocation,0,0},
  {FIELDS_DataQueryPdu,18,0},
  {FIELDS_BurstDescriptor,0,0},
  {FIELDS_LinearObjectStatePdu,44,0},
  {FIELDS_CreateEntityPdu,11,0},
  {FIELDS_RadioCommunicationsFamilyPdu,0,4},
  {FIELDS_AcousticBeamData,0,0},
  {FIELDS_IntercomSignalPdu,31,0},
  {FIELDS_GridAxisRecordRepresentation2,0,0},
  {FIELDS_LinearSegmentParameter,0,0},
  {FIELDS_GridAxisRecordRepresentation1,0,0},
  {FIELDS_GridAxisRecordRepresentation0,0,0},
  {FIELDS_RemoveEntityPdu,12,0},
  {FIELDS_ResupplyReceivedPdu,7,0},
  {FIELDS_WarfareFamilyPdu,0,2},
  {FIELDS_ElectronicEmissionSystemData,0,0},
  {FIELDS_ActionRequestPdu,16,0},
  {FIELDS_SupplyQuantity,0,0},
  {FIELDS_AcknowledgePdu,15,0},
  {FIELDS_DistributedEmissionsFamilyPdu,0,6},
  {FIELDS_IffAtcNavAidsLayer1Pdu,28,0},
  {FIELDS_SimulationManagementWithReliabilityFamilyPdu,0,10},
  {FIELDS_ActionRequestReliablePdu,56,0},
  {FIELDS_DesignatorPdu,24,0},
  {FIELDS_GriddedDataPdu,42,0},
  {FIELDS_SetRecordReliablePdu,64,0},
  {FIELDS_StopFreezePdu,14,0},
  {FIELDS_ResupplyCancelPdu,8,0},
  {FIELDS_EntityManagementFamilyPdu,0,7},
  {FIELDS_StartResumePdu,13,0},
  {FIELDS_TransmitterPdu,25,0},
  {FIELDS_TrackJamTarget,0,0},
  {FIELDS_ElectronicEmissionsPdu,23,0},
  {FIELDS_ResupplyOfferPdu,6,0},
  {FIELDS_MinefieldFamilyPdu,0,8},
  {FIELDS_SetDataReliablePdu,59,0},
  {FIELDS_EventReportPdu,21,0},
  {FIELDS_PointObjectStatePdu,43,0},
  {FIELDS_EnvironmentalProcessPdu,41,0},
  {FIELDS_DataPdu,20,0},
  {FIELDS_IsGroupOfPdu,34,0},
  {FIELDS_MinefieldDataPdu,39,0},
  {FIELDS_TransferControlRequestPdu,35,0},
  {FIELDS_EntityInformationFamilyPdu,0,1},
  {FIELDS_AcknowledgeReliablePdu,55,0},
  {FIELDS_StartResumeReliablePdu,53,0},
  {FIELDS_IffAtcNavAidsLayer2Pdu,0,0},
  {FIELDS_ArealObjectStatePdu,45,0},
  {FIELDS_DataQueryReliablePdu,58,0},
  {FIELDS_AggregateStatePdu,33,0},
  {FIELDS_EntityStateUpdatePdu,67,1},
  {FIELDS_MinefieldStatePdu,37,0},
  {FIELDS_DataReliablePdu,60,0},
  {FIELDS_CommentPdu,22,0},
  {FIELDS_CommentReliablePdu,62,0},
  {FIELDS_DetonationPdu,3,0},
  {FIELDS_SetDataPdu,19,0},
  {FIELDS_RecordQueryReliablePdu,63,0},
  {FIELDS_CollisionPdu,4,1},
  {FIELDS_ActionResponsePdu,17,0},
  {FIELDS_FirePdu,2,0},
  {FIELDS_ReceiverPdu,27,0},
  {FIELDS_UaPdu,29,0},
  {FIELDS_IntercomControlPdu,32,0},
  {FIELDS_SignalPdu,26,0},
  {FIELDS_RemoveEntityReliablePdu,52,0},
  {FIELDS_SeesPdu,30,0},
  {FIELDS_CreateEntityReliablePdu,51,0},
  {FIELDS_StopFreezeReliablePdu,54,0},
  {FIELDS_EventReportReliablePdu,61,0},
  {FIELDS_MinefieldResponseNackPdu,40,0},
  {FIELDS_CollisionElasticPdu,66,1},
  {FIELDS_ActionResponseReliablePdu,57,0},
  {FIELDS_IsPartOfPdu,36,0},
  {FIELDS_MinefieldQueryPdu,38,0},
  {FIELDS_EntityStatePdu,1,0},
  {FIELDS_FastEntityStatePdu,1,0},
};
int pdu2dis [] = {
  0,
  type_FastEntityStatePdu,
  type_FirePdu,
  type_DetonationPdu,
  type_CollisionPdu,
  type_ServiceRequestPdu,
  type_ResupplyOfferPdu,
  type_ResupplyReceivedPdu,
  type_ResupplyCancelPdu,
  type_RepairCompletePdu,
  type_RepairResponsePdu,
  type_CreateEntityPdu,
  type_RemoveEntityPdu,
  type_StartResumePdu,
  type_StopFreezePdu,
  type_AcknowledgePdu,
  type_ActionRequestPdu,
  type_ActionResponsePdu,
  type_DataQueryPdu,
  type_SetDataPdu,
  type_DataPdu,
  type_EventReportPdu,
  type_CommentPdu,
  type_ElectronicEmissionsPdu,
  type_DesignatorPdu,
  type_TransmitterPdu,
  type_SignalPdu,
  type_ReceiverPdu,
  type_IffAtcNavAidsLayer1Pdu,
  type_UaPdu,
  type_SeesPdu,
  type_IntercomSignalPdu,
  type_IntercomControlPdu,
  type_AggregateStatePdu,
  type_IsGroupOfPdu,
  type_TransferControlRequestPdu,
  type_IsPartOfPdu,
  type_MinefieldStatePdu,
  type_MinefieldQueryPdu,
  type_MinefieldDataPdu,
  type_MinefieldResponseNackPdu,
  type_EnvironmentalProcessPdu,
  type_GriddedDataPdu,
  type_PointObjectStatePdu,
  type_LinearObjectStatePdu,
  type_ArealObjectStatePdu,
  0,
  0,
  0,
  0,
  0,
  type_CreateEntityReliablePdu,
  type_RemoveEntityReliablePdu,
  type_StartResumeReliablePdu,
  type_StopFreezeReliablePdu,
  type_AcknowledgeReliablePdu,
  type_ActionRequestReliablePdu,
  type_ActionResponseReliablePdu,
  type_DataQueryReliablePdu,
  type_SetDataReliablePdu,
  type_DataReliablePdu,
  type_EventReportReliablePdu,
  type_CommentReliablePdu,
  type_RecordQueryReliablePdu,
  type_SetRecordReliablePdu,
  0,
  type_CollisionElasticPdu,
};
int pduToDis(int pdu){
  return pdu2dis[pdu];
}
void initializeclass(void *t, int distype);
void initialize_field(char *t,struct disfieldattr* field){
    switch(field->kind){
        case CLASSREF:
            initializeclass(t,field->type);
            break;
        case PRIMITIVE:
           switch(field->type){
           case type_UBYTE: memcpy(t,&(unsigned char)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_BYTE: memcpy(t,&(char)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_USHORT: memcpy(t,&(unsigned short)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_SHORT: memcpy(t,&(short)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_UINT: memcpy(t,&(unsigned int)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_INT: memcpy(t,&(int)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_ULONG: memcpy(t,&(unsigned long long)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_LONG: memcpy(t,&(long long)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_FLOAT: memcpy(t,&(float)field->defaultvalue,TYPE_SIZE[field->type]); break;
           case type_DOUBLE: memcpy(t,&(double)field->defaultvalue,TYPE_SIZE[field->type]); break;
           default: break;
           }
            break;
        case FIXED_LIST:
        case VARIABLE_LIST:
           break;
    }
}
void initializeclass(char *t, int distype){
    struct disfieldattr *field;
    memset(t,0,TYPE_SIZE[distype]);
    field = DIS_CLASS[distype].fields;
    do{
        //initialize field
        initialize_field(t+field->offset,field);
        field++;
    } while(field->kind > -1);
}
unsigned char * dis_ctor(int distype){
    unsigned char *item;
    struct dis_class *cinit = &DIS_CLASS[distype];
    item = malloc(TYPE_SIZE[distype]);
    memset(item,0,TYPE_SIZE[distype]);
    initializeclass(item,distype);
    if(cinit->pduType || cinit->protocolFamily)
         ctor_pduHeader(item,cinit->pduType,cinit->protocolFamily);
    return item;
}
void dis_dtor(unsigned char *item, int distype){
   struct dis_class *cinit = &DIS_CLASS[distype];
   if(item){
       //look for and free malloced VARIABLE_LIST fields
       if(cinit->fields){
           struct disfieldattr *field;
           field = cinit->fields;
           do{
               if(field->kind == VARIABLE_LIST){
                   unsigned char **vlist;
                   vlist = (unsigned char **)(item + field->offset);
                   if(*vlist)
                       free(*vlist);
               }
               field++;
           } while(field->kind > -1);
       }
       //free disdata
       free(item);
   }
}

static int isHostLittleEndian = TRUE; //set once during program run 
static int unde = 0; //debug = 1
char *endianswap(char *target, char *source, int size){ 
    if(isHostLittleEndian){ 
        int i; 
        for(i=0;i<size;i++) 
            target[i] = source[size-i-1]; 
    }else{ 
        memcpy(target,source,size); 
    } 
    return target; 
 } 
int primitive2int(unsigned char *ptr, int type){
    int n = 0;
    switch(type){
        case type_BYTE:
            n = (int)*(unsigned char*)ptr; break;
        case type_UBYTE:
            n = (int)*(char *)ptr; break;
        case type_INT:
            n = (int)*(int*)ptr; break;
        case type_UINT:
            n = (int)*(unsigned int *)ptr; break;
        case type_LONG:
            n = (int)*(long long *)ptr; break;
        case type_ULONG:
            n = (int)*(unsigned long long*)ptr; break;
        default:
            n = 0;
    }
    return n;
}
unsigned char *dis_marshal_list_item(unsigned char *datastream, unsigned char* item, int kind, int type){
   unsigned char *carat; 
   int size;
   carat = datastream; 
   size = TYPE_SIZE[type];
   switch(kind){ 
       case CLASSREF:
           //recurse for complex types 
           if(unde) printf("classref size %d\n",size*8);
           carat = dis_marshal(carat, item, type); 
           if(unde) printf("/classref\n");
           break; 
       case PRIMITIVE: 
           endianswap(carat,item, size); 
           if(unde) printf("doing primitive size %d\n",size*8);
           carat += size; //size in bytes 
           break; 
       default: 
           //charater and bit buffers 
           memcpy(carat,item,size); 
           carat += size; //size in bytes 
           if(unde) printf("doing unknown size %d\n",size * 8);
           break; 
       }
   return carat;
}
unsigned char * dis_marshal(unsigned char * datastream, unsigned char *item, int type) 
{ 
    int i,n; 
    unsigned char *carat; 
    unsigned char **vlist; 
    struct disfieldattr *field = DIS_CLASS[type].fields; 
    carat = datastream; 
    do{ 
        switch(field->kind){ 
            case CLASSREF: 
                //recurse for complex types 
                carat = dis_marshal(carat,item + field->offset, field->type); 
                break; 
            case PRIMITIVE: 
                endianswap(carat,item + field->offset,field->size); 
                carat += field->size; //size in bytes  
                break; 
            case FIXED_LIST: 
                //in-place list 
                for(i=0;i<field->listLength;i++){ 
                   carat = dis_marshal_list_item(carat,item + field->offset + i*field->size,field->listkind,field->type); 
                } 
                break; 
            case VARIABLE_LIST: 
                n = 0;
                {
                   struct disfieldattr *field2 = &DIS_CLASS[type].fields[field->countfieldindex];
                   n = primitive2int(item+field2->offset,field2->type);
                }
                vlist = (unsigned char **)(item + field->offset);
                for(i=0;i<n;i++){
                   carat = dis_marshal_list_item(carat,*vlist + i*field->size,field->listkind,field->type);
                }
                if(unde) printf("doing %s variable list size %d\n",field->name,field->size * field->listLength * 8);
                break; 
            default: 
                //charater and bit buffers 
                memcpy(carat,item + field->offset,field->size); 
                carat += field->size; //size in bytes 
                break; 
        } 
        field ++; 
    }while(field->kind > -1); 
    return carat; 
}
unsigned char *dis_unmarshal_list_item(unsigned char *datastream, unsigned char* item, int kind, int type){
    unsigned char *carat; 
    int size;
    carat = datastream; 
    size = TYPE_SIZE[type];
    switch(kind){ 
        case CLASSREF:
            //recurse for complex types 
            if(unde) printf("classref size %d\n",size*8);
            carat = dis_unmarshal(carat, item, type); 
            if(unde) printf("/classref\n");
            break; 
        case PRIMITIVE: 
            endianswap(item,carat, size); 
            if(unde) printf("doing primitive size %d\n",size*8);
            carat += size; //size in bytes 
            break; 
        default: 
            //charater and bit buffers 
            memcpy(item,carat,size); 
            carat += size; //size in bytes 
            if(unde) printf("doing unknown size %d\n",size * 8);
            break; 
    }
    return carat;
}
unsigned char *dis_unmarshal(unsigned char *datastream, unsigned char* item, int type){
    unsigned char *carat; 
    unsigned char **vlist; 
    int i, n; 
    struct disfieldattr *field = DIS_CLASS[type].fields; 
    carat = datastream; 
    do{ 
        switch(field->kind){ 
            case CLASSREF: 
                //recurse for complex types 
                if(unde) printf("classref %s size %d\n",field->name,field->size*8);
                carat = dis_unmarshal(carat,item + field->offset, field->type); 
                if(unde) printf("/classref\n");
                break; 
            case PRIMITIVE: 
                endianswap(item + field->offset,carat, field->size); 
                if(unde) printf("doing %s primitive size %d\n",field->name,field->size*8);
                carat += field->size; //size in bytes 
                break; 
            case FIXED_LIST: 
                //in-place list 
                for(i=0;i<field->listLength;i++){ 
                    carat = dis_unmarshal_list_item(carat,item + field->offset + i*field->size,field->listkind,field->type);
                }
                if(unde) printf("doing %s fixed list size %d\n",field->name,field->size*field->listLength*8);
                break; 
            case VARIABLE_LIST: 
                n = 0;
                {
                   struct disfieldattr *field2 = &DIS_CLASS[type].fields[field->countfieldindex];
                   n = primitive2int(item+field2->offset,field2->type);
                }
                vlist = (unsigned char **)(item + field->offset);
                *vlist = NULL;
                if(n) *vlist = malloc(n * field->size);
                for(i=0;i<n;i++){
                  carat = dis_unmarshal_list_item(carat,*vlist + i*field->size,field->listkind,field->type); 
                }
                if(unde) printf("doing %s variable list size %d\n",field->name,field->size * field->listLength * 8);
                break; 
            default: 
                //charater and bit buffers 
                memcpy(item + field->offset,carat,field->size); 
                carat += field->size; //size in bytes 
                if(unde) printf("doing %s unknown size %d\n",field->name,field->size * 8);
                break; 
        } 
        field ++; 
    }while(field->kind > -1); 
    return carat;
}

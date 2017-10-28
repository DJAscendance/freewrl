#ifndef DIS_H
#define DIS_H

#ifdef __cplusplus
extern "C" {
#endif

enum {
  type_UINT = 0,
  type_UBYTE,
  type_BYTE,
  type_DOUBLE,
  type_SHORT,
  type_ULONG,
  type_FLOAT,
  type_USHORT,
  type_INT,
  type_LONG,
  type_SystemID,
  type_RadioEntityType,
  type_LayerHeader,
  type_AcousticEmitterSystem,
  type_FourByteChunk,
  type_Orientation,
  type_OneByteChunk,
  type_EventID,
  type_VectoringNozzleSystemData,
  type_ObjectType,
  type_FundamentalParameterDataIff,
  type_EightByteChunk,
  type_FixedDatum,
  type_GridAxisRecord,
  type_AggregateID,
  type_TwoByteChunk,
  type_ClockTime,
  type_Relationship,
  type_Vector3Float,
  type_ModulationType,
  type_SimulationAddress,
  type_IffFundamentalData,
  type_AggregateType,
  type_BeamData,
  type_NamedLocation,
  type_RecordSet,
  type_SphericalHarmonicAntennaPattern,
  type_ShaftRPMs,
  type_IntercomCommunicationsParameters,
  type_AcousticBeamFundamentalParameter,
  type_EntityType,
  type_FundamentalParameterData,
  type_ApaData,
  type_Environment,
  type_AcousticEmitter,
  type_AngularVelocityVector,
  type_AggregateMarking,
  type_EntityID,
  type_SixByteChunk,
  type_Vector3Double,
  type_Pdu,
  type_VariableDatum,
  type_ArticulationParameter,
  type_Marking,
  type_Point,
  type_PropulsionSystemData,
  type_EmitterSystem,
  type_PduContainer,
  type_ElectronicEmissionBeamData,
  type_LogisticsFamilyPdu,
  type_ServiceRequestPdu,
  type_RepairCompletePdu,
  type_DeadReckoningParameter,
  type_BeamAntennaPattern,
  type_SyntheticEnvironmentFamilyPdu,
  type_AcousticEmitterSystemData,
  type_RepairResponsePdu,
  type_SimulationManagementFamilyPdu,
  type_AntennaLocation,
  type_DataQueryPdu,
  type_BurstDescriptor,
  type_LinearObjectStatePdu,
  type_CreateEntityPdu,
  type_RadioCommunicationsFamilyPdu,
  type_AcousticBeamData,
  type_IntercomSignalPdu,
  type_GridAxisRecordRepresentation2,
  type_LinearSegmentParameter,
  type_GridAxisRecordRepresentation1,
  type_GridAxisRecordRepresentation0,
  type_RemoveEntityPdu,
  type_ResupplyReceivedPdu,
  type_WarfareFamilyPdu,
  type_ElectronicEmissionSystemData,
  type_ActionRequestPdu,
  type_SupplyQuantity,
  type_AcknowledgePdu,
  type_DistributedEmissionsFamilyPdu,
  type_IffAtcNavAidsLayer1Pdu,
  type_SimulationManagementWithReliabilityFamilyPdu,
  type_ActionRequestReliablePdu,
  type_DesignatorPdu,
  type_GriddedDataPdu,
  type_SetRecordReliablePdu,
  type_StopFreezePdu,
  type_ResupplyCancelPdu,
  type_EntityManagementFamilyPdu,
  type_StartResumePdu,
  type_TransmitterPdu,
  type_TrackJamTarget,
  type_ElectronicEmissionsPdu,
  type_ResupplyOfferPdu,
  type_MinefieldFamilyPdu,
  type_SetDataReliablePdu,
  type_EventReportPdu,
  type_PointObjectStatePdu,
  type_EnvironmentalProcessPdu,
  type_DataPdu,
  type_IsGroupOfPdu,
  type_MinefieldDataPdu,
  type_TransferControlRequestPdu,
  type_EntityInformationFamilyPdu,
  type_AcknowledgeReliablePdu,
  type_StartResumeReliablePdu,
  type_IffAtcNavAidsLayer2Pdu,
  type_ArealObjectStatePdu,
  type_DataQueryReliablePdu,
  type_AggregateStatePdu,
  type_EntityStateUpdatePdu,
  type_MinefieldStatePdu,
  type_DataReliablePdu,
  type_CommentPdu,
  type_CommentReliablePdu,
  type_DetonationPdu,
  type_SetDataPdu,
  type_RecordQueryReliablePdu,
  type_CollisionPdu,
  type_ActionResponsePdu,
  type_FirePdu,
  type_ReceiverPdu,
  type_UaPdu,
  type_IntercomControlPdu,
  type_SignalPdu,
  type_RemoveEntityReliablePdu,
  type_SeesPdu,
  type_CreateEntityReliablePdu,
  type_StopFreezeReliablePdu,
  type_EventReportReliablePdu,
  type_MinefieldResponseNackPdu,
  type_CollisionElasticPdu,
  type_ActionResponseReliablePdu,
  type_IsPartOfPdu,
  type_MinefieldQueryPdu,
  type_EntityStatePdu,
  type_FastEntityStatePdu,
};
/* 5.2.58. Used in IFF ATC PDU */
struct SystemID{
  /** System Type */
  unsigned short systemType; 
  /** System name, an enumeration */
  unsigned short systemName; 
  /** System mode */
  unsigned char systemMode; 
  /** Change Options */
  unsigned char changeOptions; 
};

/* Section 5.2.25. Identifies the type of radio */
struct RadioEntityType{
  /** Kind of entity */
  unsigned char entityKind; 
  /** Domain of entity (air, surface, subsurface, space, etc) */
  unsigned char domain; 
  /** country to which the design of the entity is attributed */
  unsigned short country; 
  /** category of entity */
  unsigned char category; 
  /** specific info based on subcategory field */
  unsigned char nomenclatureVersion; 
  unsigned short nomenclature; 
};

/* 5.2.47.  Layer header. */
struct LayerHeader{
  /** Layer number */
  unsigned char layerNumber; 
  /** Layer speccific information enumeration */
  unsigned char layerSpecificInformaiton; 
  /** information length */
  unsigned short length; 
};

/* 5.3.35: Information about a particular UA emitter shall be represented using an Acoustic Emitter System record. This record shall consist of three fields: Acoustic Name, Function, and Acoustic ID Number */
struct AcousticEmitterSystem{
  /** This field shall specify the system for a particular UA emitter. */
  unsigned short acousticName; 
  /** This field shall describe the function of the acoustic system.  */
  unsigned char acousticFunction; 
  /** This field shall specify the UA emitter identification number relative to a specific system. This field shall be represented by an 8-bit unsigned integer. This field allows the differentiation of multiple systems on an entity, even if in some instances two or more of the systems may be identical UA emitter types. Numbering of systems shall begin with the value 1.  */
  unsigned char acousticID; 
};

/* 32 bit piece of data */
struct FourByteChunk{
  /** four bytes of arbitrary data */
  char otherParameters[4]; 
};

/* Section 5.2.17. Three floating point values representing an orientation, psi, theta, and phi, aka the euler angles, in radians */
struct Orientation{
  float psi; 
  float theta; 
  float phi; 
};

/* 8 bit piece of data */
struct OneByteChunk{
  /** one byte of arbitrary data */
  char otherParameters[1]; 
};

/* Section 5.2.18. Identifies a unique event in a simulation via the combination of three values */
struct EventID{
  /** The site ID */
  unsigned short site; 
  /** The application ID */
  unsigned short application; 
  /** the number of the event */
  unsigned short eventNumber; 
};

/* Data about a vectoring nozzle system */
struct VectoringNozzleSystemData{
  /** horizontal deflection angle */
  float horizontalDeflectionAngle; 
  /** vertical deflection angle */
  float verticalDeflectionAngle; 
};

/* Identifies type of object. This is a shorter version of EntityType that omits the specific and extra fields. */
struct ObjectType{
  /** Kind of entity */
  unsigned char entityKind; 
  /** Domain of entity (air, surface, subsurface, space, etc) */
  unsigned char domain; 
  /** country to which the design of the entity is attributed */
  unsigned short country; 
  /** category of entity */
  unsigned char category; 
  /** subcategory of entity */
  unsigned char subcategory; 
};

/* 5.2.45. Fundamental IFF atc data */
struct FundamentalParameterDataIff{
  /** ERP */
  float erp; 
  /** frequency */
  float frequency; 
  /** pgrf */
  float pgrf; 
  /** Pulse width */
  float pulseWidth; 
  /** Burst length */
  unsigned int burstLength; 
  /** Applicable modes enumeration */
  unsigned char applicableModes; 
  /** padding */
  unsigned short pad2; 
  /** padding */
  unsigned char pad3; 
};

/* 64 bit piece of data */
struct EightByteChunk{
  /** Eight bytes of arbitrary data */
  char otherParameters[8]; 
};

/* Section 5.2.18. Fixed Datum Record */
struct FixedDatum{
  /** ID of the fixed datum */
  unsigned int fixedDatumID; 
  /** Value for the fixed datum */
  unsigned int fixedDatumValue; 
};

/* 5.2.44: Grid data record, a common abstract superclass for several subtypes  */
struct GridAxisRecord{
  /** type of environmental sample */
  unsigned short sampleType; 
  /** value that describes data representation */
  unsigned short dataRepresentation; 
};

/* Section 5.2.36. Each agregate in a given simulation app is given an aggregate identifier number unique for all other aggregates in that app and in that exercise. The id is valid for the duration of the the exercise. */
struct AggregateID{
  /** The site ID */
  unsigned short site; 
  /** The application ID */
  unsigned short application; 
  /** the aggregate ID */
  unsigned short aggregateID; 
};

/* 16 bit piece of data */
struct TwoByteChunk{
  /** two bytes of arbitrary data */
  char otherParameters[2]; 
};

/* Section 5.2.8. Time measurements that exceed one hour. Hours is the number of           hours since January 1, 1970, UTC */
struct ClockTime{
  /** Hours in UTC */
  int hour; 
  /** Time past the hour */
  unsigned int timePastHour; 
};

/* 5.2.56. Purpose for joinging two entities */
struct Relationship{
  /** Nature of join */
  unsigned short nature; 
  /** position of join */
  unsigned short position; 
};

/* Section 5.2.33. Three floating point values, x, y, and z */
struct Vector3Float{
  /** X value */
  float x; 
  /** y Value */
  float y; 
  /** Z value */
  float z; 
};

/* Radio modulation */
struct ModulationType{
  /** spread spectrum, 16 bit boolean array */
  unsigned short spreadSpectrum; 
  /** major */
  unsigned short major; 
  /** detail */
  unsigned short detail; 
  /** system */
  unsigned short system; 
};

/* Section 5.2.14.1. A Simulation Address  record shall consist of the Site Identification number and the Application Identification number. */
struct SimulationAddress{
  /** The site ID */
  unsigned short site; 
  /** The application ID */
  unsigned short application; 
};

/* 5.2.42. Basic operational data ofr IFF ATC NAVAIDS */
struct IffFundamentalData{
  /** system status */
  unsigned char systemStatus; 
  /** Alternate parameter 4 */
  unsigned char alternateParameter4; 
  /** eight boolean fields */
  unsigned char informationLayers; 
  /** enumeration */
  unsigned char modifier; 
  /** parameter, enumeration */
  unsigned short parameter1; 
  /** parameter, enumeration */
  unsigned short parameter2; 
  /** parameter, enumeration */
  unsigned short parameter3; 
  /** parameter, enumeration */
  unsigned short parameter4; 
  /** parameter, enumeration */
  unsigned short parameter5; 
  /** parameter, enumeration */
  unsigned short parameter6; 
};

/* Section 5.2.38. Identifies the type of aggregate including kind of entity, domain (surface, subsurface, air, etc) country, category, etc. */
struct AggregateType{
  /** Kind of entity */
  unsigned char aggregateKind; 
  /** Domain of entity (air, surface, subsurface, space, etc) */
  unsigned char domain; 
  /** country to which the design of the entity is attributed */
  unsigned short country; 
  /** category of entity */
  unsigned char category; 
  /** subcategory of entity */
  unsigned char subcategory; 
  /** specific info based on subcategory field */
  unsigned char specific; 
  unsigned char extra; 
};

/* Section 5.2.39. Specification of the data necessary to  describe the scan volume of an emitter. */
struct BeamData{
  /** Specifies the beam azimuth an elevation centers and corresponding half-angles     to describe the scan volume */
  float beamAzimuthCenter; 
  /** Specifies the beam azimuth sweep to determine scan volume */
  float beamAzimuthSweep; 
  /** Specifies the beam elevation center to determine scan volume */
  float beamElevationCenter; 
  /** Specifies the beam elevation sweep to determine scan volume */
  float beamElevationSweep; 
  /** allows receiver to synchronize its regenerated scan pattern to     that of the emmitter. Specifies the percentage of time a scan is through its pattern from its origion. */
  float beamSweepSync; 
};

/* discrete ostional relationsihip  */
struct NamedLocation{
  /** station name enumeration */
  unsigned short stationName; 
  /** station number */
  unsigned short stationNumber; 
};

/* Record sets, used in transfer control request PDU */
struct RecordSet{
  /** record ID */
  unsigned int recordID; 
  /** record set serial number */
  unsigned int recordSetSerialNumber; 
  /** record length */
  unsigned short recordLength; 
  /** record count */
  unsigned short recordCount; 
  /** ^^^This is wrong--variable sized data records */
  unsigned short recordValues; 
  /** ^^^This is wrong--variable sized padding */
  unsigned char pad4; 
};

/* Section 5.2.4.3. Used when the antenna pattern type in the transmitter pdu is of value 2.         Specified the direction and radiation pattern from a radio transmitter's antenna.        NOTE: this class must be hand-coded to clean up some implementation details. */
struct SphericalHarmonicAntennaPattern{
  char order; 
};

/* Shaft RPMs, used in underwater acoustic clacluations. */
struct ShaftRPMs{
  /** Current shaft RPMs */
  short currentShaftRPMs; 
  /** ordered shaft rpms */
  short orderedShaftRPMs; 
  /** rate of change of shaft RPMs */
  float shaftRPMRateOfChange; 
};

/* 5.2.46.  Intercom communcations parameters */
struct IntercomCommunicationsParameters{
  /** Type of intercom parameters record */
  unsigned short recordType; 
  /** length of record */
  unsigned short recordLength; 
  /** Jerks. Looks like the committee is forcing a lookup of the record type parameter to find out how long the field is. This is a placeholder. */
  unsigned int recordSpecificField; 
};

/* Used in UaPdu */
struct AcousticBeamFundamentalParameter{
  /** parameter index */
  unsigned short activeEmissionParameterIndex; 
  /** scan pattern */
  unsigned short scanPattern; 
  /** beam center azimuth */
  float beamCenterAzimuth; 
  /** azimuthal beamwidth */
  float azimuthalBeamwidth; 
  /** beam center */
  float beamCenterDE; 
  /** DE beamwidth (vertical beamwidth) */
  float deBeamwidth; 
};

/* Section 5.2.16. Identifies the type of entity, including kind of entity, domain (surface, subsurface, air, etc) country, category, etc. */
struct EntityType{
  /** Kind of entity */
  unsigned char entityKind; 
  /** Domain of entity (air, surface, subsurface, space, etc) */
  unsigned char domain; 
  /** country to which the design of the entity is attributed */
  unsigned short country; 
  /** category of entity */
  unsigned char category; 
  /** subcategory of entity */
  unsigned char subcategory; 
  /** specific info based on subcategory field */
  unsigned char specific; 
  unsigned char extra; 
};

/* Section 5.2.22. Contains electromagnetic emmision regineratin parameters that are        variable throughout a scenario dependent on the actions of the participants in the simulation. Also provides basic parametric data that may be used to support low-fidelity simulations. */
struct FundamentalParameterData{
  /** center frequency of the emission in hertz. */
  float frequency; 
  /** Bandwidth of the frequencies corresponding to the fequency field. */
  float frequencyRange; 
  /** Effective radiated power for the emission in DdBm. For a      radar noise jammer, indicates the peak of the transmitted power. */
  float effectiveRadiatedPower; 
  /** Average repetition frequency of the emission in hertz. */
  float pulseRepetitionFrequency; 
  /** Average pulse width  of the emission in microseconds. */
  float pulseWidth; 
  /** Specifies the beam azimuth an elevation centers and corresponding half-angles     to describe the scan volume */
  float beamAzimuthCenter; 
  /** Specifies the beam azimuth sweep to determine scan volume */
  float beamAzimuthSweep; 
  /** Specifies the beam elevation center to determine scan volume */
  float beamElevationCenter; 
  /** Specifies the beam elevation sweep to determine scan volume */
  float beamElevationSweep; 
  /** allows receiver to synchronize its regenerated scan pattern to     that of the emmitter. Specifies the percentage of time a scan is through its pattern from its origion. */
  float beamSweepSync; 
};

/* Used in UA PDU */
struct ApaData{
  /** Index of APA parameter */
  unsigned short parameterIndex; 
  /** Index of APA parameter */
  short parameterValue; 
};

/* Section 5.2.40. Information about a geometry, a state associated with a geometry, a bounding volume, or an associated entity ID. NOTE: this class requires hand coding. */
struct Environment{
  /** Record type */
  unsigned int environmentType; 
  /** length, in bits */
  unsigned char length; 
  /** Identify the sequentially numbered record index */
  unsigned char index; 
  /** padding */
  unsigned char padding1; 
  /** Geometry or state record */
  unsigned char geometry; 
  /** padding to bring the total size up to a 64 bit boundry */
  unsigned char padding2; 
};

/* Section 5.2.35. information about a specific UA emmtter */
struct AcousticEmitter{
  /** the system for a particular UA emitter, and an enumeration */
  unsigned short acousticName; 
  /** The function of the acoustic system */
  unsigned char function; 
  /** The UA emitter identification number relative to a specific system */
  unsigned char acousticIdNumber; 
};

/* 5.2.2: angular velocity measured in radians per second out each of the entity's own coordinate axes. */
struct AngularVelocityVector{
  /** velocity about the x axis */
  float x; 
  /** velocity about the y axis */
  float y; 
  /** velocity about the zaxis */
  float z; 
};

/* Section 5.2.37. Specifies the character set used inthe first byte, followed by up to 31 characters of text data. */
struct AggregateMarking{
  /** The character set */
  unsigned char characterSet; 
  /** The characters */
  char characters[31]; 
};

/* Each entity in a given DIS simulation application shall be given an entity identifier number unique to all  other entities in that application. This identifier number is valid for the duration of the exercise; however,  entity identifier numbers may be reused when all possible numbers have been exhausted. No entity shall  have an entity identifier number of NO_ENTITY, ALL_ENTITIES, or RQST_ASSIGN_ID. The entity iden-  tifier number need not be registered or retained for future exercises. The entity identifier number shall be  specified by a 16-bit unsigned integer.  An entity identifier number equal to zero with valid site and application identification shall address a  simulation application. An entity identifier number equal to ALL_ENTITIES shall mean all entities within  the specified site and application. An entity identifier number equal to RQST_ASSIGN_ID allows the  receiver of the create entity to define the entity identifier number of the new entity. */
struct EntityID{
  /** The site ID */
  unsigned short site; 
  /** The application ID */
  unsigned short application; 
  /** the entity ID */
  unsigned short entity; 
};

/* 48 bit piece of data */
struct SixByteChunk{
  /** six bytes of arbitrary data */
  char otherParameters[6]; 
};

/* Section 5.3.34. Three double precision floating point values, x, y, and z */
struct Vector3Double{
  /** X value */
  double x; 
  /** Y value */
  double y; 
  /** Z value */
  double z; 
};

/* The superclass for all PDUs. This incorporates the PduHeader record, section 5.2.29. */
struct Pdu{
  /** The version of the protocol. 5=DIS-1995, 6=DIS-1998. */
  unsigned char protocolVersion; 
  /** Exercise ID */
  unsigned char exerciseID; 
  /** Type of pdu, unique for each PDU class */
  unsigned char pduType; 
  /** value that refers to the protocol family, eg SimulationManagement, et */
  unsigned char protocolFamily; 
  /** Timestamp value */
  unsigned int timestamp; 
  /** Length, in bytes, of the PDU */
  unsigned short length; 
  /** zero-filled array of padding */
  short padding; 
};

/* Section 5.2.32. Variable Datum Record */
struct VariableDatum{
  /** ID of the variable datum */
  unsigned int variableDatumID; 
  /** length of the variable datums */
  unsigned int variableDatumLength; 
  /** variable length list of 64-bit datums */
  void * variableDatums; 
};

/* Section 5.2.5. Articulation parameters for  movable parts and attached parts of an entity. Specifes wether or not a change has occured,  the part identifcation of the articulated part to which it is attached, and the type and value of each parameter. */
struct ArticulationParameter{
  unsigned char parameterTypeDesignator; 
  unsigned char changeIndicator; 
  unsigned short partAttachedTo; 
  int parameterType; 
  double parameterValue; 
};

/* Section 5.2.15. Specifies the character set used inthe first byte, followed by 11 characters of text data. */
struct Marking{
  /** The character set */
  unsigned char characterSet; 
  /** The characters */
  char characters[11]; 
};

/* x,y point */
struct Point{
  /** x */
  float x; 
  /** y */
  float y; 
};

/* Data about a propulsion system */
struct PropulsionSystemData{
  /** powerSetting */
  float powerSetting; 
  /** engine RPMs */
  float engineRpm; 
};

/* Section 5.2.11. This field shall specify information about a particular emitter system */
struct EmitterSystem{
  /** Name of the emitter, 16 bit enumeration */
  unsigned short emitterName; 
  /** function of the emitter, 8 bit enumeration */
  unsigned char function; 
  /** emitter ID, 8 bit enumeration */
  unsigned char emitterIdNumber; 
};

/* Used for XML compatability. A container that holds PDUs */
struct PduContainer{
  /** Number of PDUs in the container list */
  int numberOfPdus; 
  /** record sets */
  void * pdus; 
};

/* Description of one electronic emission beam */
struct ElectronicEmissionBeamData{
  /** This field shall specify the length of this beams data in 32 bit words */
  unsigned char beamDataLength; 
  /** This field shall specify a unique emitter database number assigned to differentiate between otherwise similar or identical emitter beams within an emitter system. */
  unsigned char beamIDNumber; 
  /** This field shall specify a Beam Parameter Index number that shall be used by receiving entities in conjunction with the Emitter Name field to provide a pointer to the stored database parameters required to regenerate the beam.  */
  unsigned short beamParameterIndex; 
  /** Fundamental parameter data such as frequency range, beam sweep, etc. */
  struct FundamentalParameterData fundamentalParameterData; 
  /** beam function of a particular beam */
  unsigned char beamFunction; 
  /** Number of track/jam targets */
  unsigned char numberOfTrackJamTargets; 
  /** wheher or not the receiving simulation apps can assume all the targets in the scan pattern are being tracked/jammed */
  unsigned char highDensityTrackJam; 
  /** padding */
  unsigned char pad4; 
  /** identify jamming techniques used */
  unsigned int jammingModeSequence; 
  /** variable length list of track/jam targets */
  void * trackJamTargets; 
};

/* Section 5.3.5. Abstract superclass for logistics PDUs. COMPLETE */
struct LogisticsFamilyPdu{
  struct Pdu myPdu;
};

/* Section 5.3.5.1. Information about a request for supplies. COMPLETE */
struct ServiceRequestPdu{
  struct LogisticsFamilyPdu myLogisticsFamilyPdu;
  /** Entity that is requesting service */
  struct EntityID requestingEntityID; 
  /** Entity that is providing the service */
  struct EntityID servicingEntityID; 
  /** type of service requested */
  unsigned char serviceTypeRequested; 
  /** How many requested */
  unsigned char numberOfSupplyTypes; 
  /** padding */
  short serviceRequestPadding; 
  void * supplies; 
};

/* Section 5.2.5.5. Repair is complete. COMPLETE */
struct RepairCompletePdu{
  struct LogisticsFamilyPdu myLogisticsFamilyPdu;
  /** Entity that is receiving service */
  struct EntityID receivingEntityID; 
  /** Entity that is supplying */
  struct EntityID repairingEntityID; 
  /** Enumeration for type of repair */
  unsigned short repair; 
  /** padding, number prevents conflict with superclass ivar name */
  short padding2; 
};

/* represents values used in dead reckoning algorithms */
struct DeadReckoningParameter{
  /** enumeration of what dead reckoning algorighm to use */
  unsigned char deadReckoningAlgorithm; 
  /** other parameters to use in the dead reckoning algorithm */
  char otherParameters[15]; 
  /** Linear acceleration of the entity */
  struct Vector3Float entityLinearAcceleration; 
  /** angular velocity of the entity */
  struct Vector3Float entityAngularVelocity; 
};

/* Section 5.2.4.2. Used when the antenna pattern type field has a value of 1. Specifies           the direction, patter, and polarization of radiation from an antenna. */
struct BeamAntennaPattern{
  /** The rotation that transformst he reference coordinate sytem     into the beam coordinate system. Either world coordinates or entity coordinates may be used as the     reference coordinate system, as specified by teh reference system field of the antenna pattern record. */
  struct Orientation beamDirection; 
  float azimuthBeamwidth; 
  float referenceSystem; 
  short padding1; 
  char padding2; 
  /** Magnigute of the z-component in beam coordinates at some arbitrary      single point in the mainbeam      and in the far field of the antenna. */
  float ez; 
  /** Magnigute of the x-component in beam coordinates at some arbitrary      single point in the mainbeam      and in the far field of the antenna. */
  float ex; 
  /** THe phase angle between Ez and Ex in radians. */
  float phase; 
};

/* Section 5.3.11: Abstract superclass for synthetic environment PDUs */
struct SyntheticEnvironmentFamilyPdu{
  struct Pdu myPdu;
};

/* Used in the UA pdu; ties together an emmitter and a location. This requires manual cleanup; the beam data should not be attached to each emitter system. */
struct AcousticEmitterSystemData{
  /** Length of emitter system data */
  unsigned char emitterSystemDataLength; 
  /** Number of beams */
  unsigned char numberOfBeams; 
  /** padding */
  unsigned short pad2; 
  /** This field shall specify the system for a particular UA emitter. */
  struct AcousticEmitterSystem acousticEmitterSystem; 
  /** Represents the location wrt the entity */
  struct Vector3Float emitterLocation; 
  /** For each beam in numberOfBeams, an emitter system. This is not right--the beam records need to be at the end of the PDU, rather than attached to each system. */
  void * beamRecords; 
};

/* Section 5.2.5.6. Sent after repair complete PDU. COMPLETE */
struct RepairResponsePdu{
  struct LogisticsFamilyPdu myLogisticsFamilyPdu;
  /** Entity that is receiving service */
  struct EntityID receivingEntityID; 
  /** Entity that is supplying */
  struct EntityID repairingEntityID; 
  /** Result of repair operation */
  unsigned char repairResult; 
  /** padding */
  short padding1; 
  /** padding */
  char padding2; 
};

/* Section 5.3.6. Abstract superclass for PDUs relating to the simulation itself. COMPLETE */
struct SimulationManagementFamilyPdu{
  struct Pdu myPdu;
  /** Entity that is sending message */
  struct EntityID originatingEntityID; 
  /** Entity that is intended to receive message */
  struct EntityID receivingEntityID; 
};

/* 5.2.3: location of the radiating portion of the antenna, specified in world coordinates and         entity coordinates. */
struct AntennaLocation{
  /** Location of the radiating portion of the antenna in world    coordinates */
  struct Vector3Double antennaLocation; 
  /** Location of the radiating portion of the antenna     in entity coordinates */
  struct Vector3Float relativeAntennaLocation; 
};

/* Section 5.3.6.8. Request for data from an entity. COMPLETE */
struct DataQueryPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** ID of request */
  unsigned int requestID; 
  /** time issues between issues of Data PDUs. Zero means send once only. */
  unsigned int timeInterval; 
  /** Number of fixed datum records */
  unsigned int numberOfFixedDatumRecords; 
  /** Number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variable length list of fixed datums */
  void * fixedDatums; 
  /** variable length list of variable length datums */
  void * variableDatums; 
};

/* Section 5.2.7. Specifies the type of muntion fired, the type of warhead, the         type of fuse, the number of rounds fired, and the rate at which the roudns are fired in         rounds per minute. */
struct BurstDescriptor{
  /** What munition was used in the burst */
  struct EntityType munition; 
  /** type of warhead */
  unsigned short warhead; 
  /** type of fuse used */
  unsigned short fuse; 
  /** how many of the munition were fired */
  unsigned short quantity; 
  /** rate at which the munition was fired */
  unsigned short rate; 
};

/* Section 5.3.11.4: Information abut the addition or modification of a synthecic enviroment object that      is anchored to the terrain with a single point and has size or orientation. COMPLETE */
struct LinearObjectStatePdu{
  struct SyntheticEnvironmentFamilyPdu mySyntheticEnvironmentFamilyPdu;
  /** Object in synthetic environment */
  struct EntityID objectID; 
  /** Object with which this point object is associated */
  struct EntityID referencedObjectID; 
  /** unique update number of each state transition of an object */
  unsigned short updateNumber; 
  /** force ID */
  unsigned char forceID; 
  /** number of linear segment parameters */
  unsigned char numberOfSegments; 
  /** requesterID */
  struct SimulationAddress requesterID; 
  /** receiver ID */
  struct SimulationAddress receivingID; 
  /** Object type */
  struct ObjectType objectType; 
  /** Linear segment parameters */
  void * linearSegmentParameters; 
};

/* Section 5.3.6.1. Create a new entity. COMPLETE */
struct CreateEntityPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** Identifier for the request */
  unsigned int requestID; 
};

/* Section 5.3.8. Abstract superclass for radio communications PDUs. */
struct RadioCommunicationsFamilyPdu{
  struct Pdu myPdu;
  /** ID of the entitythat is the source of the communication */
  struct EntityID entityId; 
  /** particular radio within an entity */
  unsigned short radioId; 
};

/* Used in UA PDU */
struct AcousticBeamData{
  /** beam data length */
  unsigned short beamDataLength; 
  /** beamIDNumber */
  unsigned char beamIDNumber; 
  /** padding */
  unsigned short pad2; 
  /** fundamental data parameters */
  struct AcousticBeamFundamentalParameter fundamentalDataParameters; 
};

/* Section 5.3.8.4. Actual transmission of intercome voice data. COMPLETE */
struct IntercomSignalPdu{
  struct RadioCommunicationsFamilyPdu myRadioCommunicationsFamilyPdu;
  /** entity ID */
  struct EntityID entityID; 
  /** ID of communications device */
  unsigned short communicationsDeviceID; 
  /** encoding scheme */
  unsigned short encodingScheme; 
  /** tactical data link type */
  unsigned short tdlType; 
  /** sample rate */
  unsigned int sampleRate; 
  /** data length */
  unsigned short dataLength; 
  /** samples */
  unsigned short samples; 
  /** data bytes */
  void * data; 
};

/* 5.2.44: Grid data record, representation 1 */
struct GridAxisRecordRepresentation2{
  struct GridAxisRecord myGridAxisRecord;
  /** number of values */
  unsigned short numberOfValues; 
  /** variable length list of data parameters ^^^this is wrong--need padding as well */
  void * dataValues; 
};

/* 5.2.48: Linear segment parameters */
struct LinearSegmentParameter{
  /** number of segments */
  unsigned char segmentNumber; 
  /** segment appearance */
  struct SixByteChunk segmentAppearance; 
  /** location */
  struct Vector3Double location; 
  /** orientation */
  struct Orientation orientation; 
  /** segmentLength */
  unsigned short segmentLength; 
  /** segmentWidth */
  unsigned short segmentWidth; 
  /** segmentHeight */
  unsigned short segmentHeight; 
  /** segment Depth */
  unsigned short segmentDepth; 
  /** segment Depth */
  unsigned int pad1; 
};

/* 5.2.44: Grid data record, representation 1 */
struct GridAxisRecordRepresentation1{
  struct GridAxisRecord myGridAxisRecord;
  /** constant scale factor */
  float fieldScale; 
  /** constant offset used to scale grid data */
  float fieldOffset; 
  /** Number of data values */
  unsigned short numberOfValues; 
  /** variable length list of data parameters ^^^this is wrong--need padding as well */
  void * dataValues; 
};

/* 5.2.44: Grid data record, representation 0 */
struct GridAxisRecordRepresentation0{
  struct GridAxisRecord myGridAxisRecord;
  /** number of bytes of environmental state data */
  unsigned short numberOfBytes; 
  /** variable length list of data parameters ^^^this is wrong--need padding as well */
  void * dataValues; 
};

/* Section 5.3.6.2. Remove an entity. COMPLETE */
struct RemoveEntityPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** Identifier for the request */
  unsigned int requestID; 
};

/* Section 5.3.5.3. Receipt of supplies is communiated. COMPLETE */
struct ResupplyReceivedPdu{
  struct LogisticsFamilyPdu myLogisticsFamilyPdu;
  /** Entity that is receiving service */
  struct EntityID receivingEntityID; 
  /** Entity that is supplying */
  struct EntityID supplyingEntityID; 
  /** how many supplies are being offered */
  unsigned char numberOfSupplyTypes; 
  /** padding */
  short padding1; 
  /** padding */
  char padding2; 
  void * supplies; 
};

/* Section 5.3.4. abstract superclass for fire and detonation pdus that have shared information. COMPLETE */
struct WarfareFamilyPdu{
  struct Pdu myPdu;
  /** ID of the entity that shot */
  struct EntityID firingEntityID; 
  /** ID of the entity that is being shot at */
  struct EntityID targetEntityID; 
};

/* Data about one electronic system */
struct ElectronicEmissionSystemData{
  /** This field shall specify the length of this emitter system's data (including beam data and its track/jam information) in 32-bit words. The length shall include the System Data Length field.  */
  unsigned char systemDataLength; 
  /** This field shall specify the number of beams being described in the current PDU for the system being described.  */
  unsigned char numberOfBeams; 
  /** padding. */
  unsigned short emissionsPadding2; 
  /** This field shall specify information about a particular emitter system */
  struct EmitterSystem emitterSystem; 
  /** Location with respect to the entity */
  struct Vector3Float location; 
  /** variable length list of beam data records */
  void * beamDataRecords; 
};

/* Section 5.3.6.6. Request from simulation manager to an entity. COMPLETE */
struct ActionRequestPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** Request ID that is unique */
  unsigned int requestID; 
  /** identifies the action being requested */
  unsigned int actionID; 
  /** Number of fixed datum records */
  unsigned int numberOfFixedDatumRecords; 
  /** Number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variable length list of fixed datums */
  void * fixedDatums; 
  /** variable length list of variable length datums */
  void * variableDatums; 
};

/* Section 5.2.30. A supply, and the amount of that supply. Similar to an entity kind but with the addition of a quantity. */
struct SupplyQuantity{
  /** Type of supply */
  struct EntityType supplyType; 
  /** quantity to be supplied */
  unsigned char quantity; 
};

/* Section 5.3.6.5. Acknowledge the receiptof a start/resume, stop/freeze, or RemoveEntityPDU. COMPLETE */
struct AcknowledgePdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** type of message being acknowledged */
  unsigned short acknowledgeFlag; 
  /** Whether or not the receiving entity was able to comply with the request */
  unsigned short responseFlag; 
  /** Request ID that is unique */
  unsigned int requestID; 
};

/* Section 5.3.7. Electronic Emissions. Abstract superclass for distirubted emissions PDU */
struct DistributedEmissionsFamilyPdu{
  struct Pdu myPdu;
};

/* 5.3.7.4.1: Navigational and IFF PDU. COMPLETE */
struct IffAtcNavAidsLayer1Pdu{
  struct DistributedEmissionsFamilyPdu myDistributedEmissionsFamilyPdu;
  /** ID of the entity that is the source of the emissions */
  struct EntityID emittingEntityId; 
  /** Number generated by the issuing simulation to associate realted events. */
  struct EventID eventID; 
  /** Location wrt entity. There is some ambugiuity in the standard here, but this is the order it is listed in the table. */
  struct Vector3Float location; 
  /** System ID information */
  struct SystemID systemID; 
  /** padding */
  unsigned short pad2; 
  /** fundamental parameters */
  struct IffFundamentalData fundamentalParameters; 
};

/* Section 5.3.12: Abstract superclass for reliable simulation management PDUs */
struct SimulationManagementWithReliabilityFamilyPdu{
  struct Pdu myPdu;
  /** Object originatig the request */
  struct EntityID originatingEntityID; 
  /** Object with which this point object is associated */
  struct EntityID receivingEntityID; 
};

/* Section 5.3.12.6: request from a simulation manager to a managed entity to perform a specified action. COMPLETE */
struct ActionRequestReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** request ID */
  unsigned int requestID; 
  /** request ID */
  unsigned int actionID; 
  /** Fixed datum record count */
  unsigned int numberOfFixedDatumRecords; 
  /** variable datum record count */
  unsigned int numberOfVariableDatumRecords; 
  /** Fixed datum records */
  void * fixedDatumRecords; 
  /** Variable datum records */
  void * variableDatumRecords; 
};

/* Section 5.3.7.2. Handles designating operations. COMPLETE */
struct DesignatorPdu{
  struct DistributedEmissionsFamilyPdu myDistributedEmissionsFamilyPdu;
  /** ID of the entity designating */
  struct EntityID designatingEntityID; 
  /** This field shall specify a unique emitter database number assigned to  differentiate between otherwise similar or identical emitter beams within an emitter system. */
  unsigned short codeName; 
  /** ID of the entity being designated */
  struct EntityID designatedEntityID; 
  /** This field shall identify the designator code being used by the designating entity  */
  unsigned short designatorCode; 
  /** This field shall identify the designator output power in watts */
  float designatorPower; 
  /** This field shall identify the designator wavelength in units of microns */
  float designatorWavelength; 
  /** designtor spot wrt the designated entity */
  struct Vector3Float designatorSpotWrtDesignated; 
  /** designtor spot wrt the designated entity */
  struct Vector3Double designatorSpotLocation; 
  /** Dead reckoning algorithm */
  char deadReckoningAlgorithm; 
  /** padding */
  unsigned short padding1; 
  /** padding */
  char padding2; 
  /** linear accelleration of entity */
  struct Vector3Float entityLinearAcceleration; 
};

/* Section 5.3.11.2: Information about globat, spatially varying enviornmental effects. This requires manual cleanup; the grid axis        records are variable sized. UNFINISHED */
struct GriddedDataPdu{
  struct SyntheticEnvironmentFamilyPdu mySyntheticEnvironmentFamilyPdu;
  /** environmental simulation application ID */
  struct EntityID environmentalSimulationApplicationID; 
  /** unique identifier for each piece of enviornmental data */
  unsigned short fieldNumber; 
  /** sequence number for the total set of PDUS used to transmit the data */
  unsigned short pduNumber; 
  /** Total number of PDUS used to transmit the data */
  unsigned short pduTotal; 
  /** coordinate system of the grid */
  unsigned short coordinateSystem; 
  /** number of grid axes for the environmental data */
  unsigned char numberOfGridAxes; 
  /** are domain grid axes identidal to those of the priveious domain update? */
  unsigned char constantGrid; 
  /** type of environment */
  struct EntityType environmentType; 
  /** orientation of the data grid */
  struct Orientation orientation; 
  /** valid time of the enviormental data sample, 64 bit unsigned int */
  long long sampleTime; 
  /** total number of all data values for all pdus for an environmental sample */
  unsigned int totalValues; 
  /** total number of data values at each grid point. */
  unsigned char vectorDimension; 
  /** padding */
  unsigned short padding1; 
  /** padding */
  unsigned char padding2; 
  /** Grid data ^^^This is wrong */
  void * gridDataList; 
};

/* Section 5.3.12.14: Initializing or changing internal parameter info. Needs manual intervention     to fix padding in recrod set PDUs. UNFINISHED */
struct SetRecordReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** request ID */
  unsigned int requestID; 
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding. The spec is unclear and contradictory here. */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** Number of record sets in list */
  unsigned int numberOfRecordSets; 
  /** record sets */
  void * recordSets; 
};

/* Section 5.2.3.4. Stop or freeze an exercise. COMPLETE */
struct StopFreezePdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** UTC time at which the simulation shall stop or freeze */
  struct ClockTime realWorldTime; 
  /** Reason the simulation was stopped or frozen */
  unsigned char reason; 
  /** Internal behavior of the simulation and its appearance while frozento the other participants */
  unsigned char frozenBehavior; 
  /** padding */
  short padding1; 
  /** Request ID that is unique */
  unsigned int requestID; 
};

/* Section 5.2.5.4. Cancel of resupply by either the receiving or supplying entity. COMPLETE */
struct ResupplyCancelPdu{
  struct LogisticsFamilyPdu myLogisticsFamilyPdu;
  /** Entity that is receiving service */
  struct EntityID receivingEntityID; 
  /** Entity that is supplying */
  struct EntityID supplyingEntityID; 
};

/* Section 5.3.9. Common superclass for EntityManagment PDUs, including aggregate state, isGroupOf, TransferControLRequest, and isPartOf */
struct EntityManagementFamilyPdu{
  struct Pdu myPdu;
};

/* Section 5.2.6.3. Start or resume an exercise. COMPLETE */
struct StartResumePdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** UTC time at which the simulation shall start or resume */
  struct ClockTime realWorldTime; 
  /** Simulation clock time at which the simulation shall start or resume */
  struct ClockTime simulationTime; 
  /** Identifier for the request */
  unsigned int requestID; 
};

/* Section 5.3.8.1. Detailed information about a radio transmitter. This PDU requires manually         written code to complete, since the modulation parameters are of variable length. UNFINISHED */
struct TransmitterPdu{
  struct RadioCommunicationsFamilyPdu myRadioCommunicationsFamilyPdu;
  /** linear accelleration of entity */
  struct RadioEntityType radioEntityType; 
  /** transmit state */
  unsigned char transmitState; 
  /** input source */
  unsigned char inputSource; 
  /** padding */
  unsigned short padding1; 
  /** Location of antenna */
  struct Vector3Double antennaLocation; 
  /** relative location of antenna */
  struct Vector3Float relativeAntennaLocation; 
  /** antenna pattern type */
  unsigned short antennaPatternType; 
  /** atenna pattern length */
  unsigned short antennaPatternCount; 
  /** frequency */
  unsigned long long frequency; 
  /** transmit frequency Bandwidth */
  float transmitFrequencyBandwidth; 
  /** transmission power */
  float power; 
  /** modulation */
  struct ModulationType modulationType; 
  /** crypto system enumeration */
  unsigned short cryptoSystem; 
  /** crypto system key identifer */
  unsigned short cryptoKeyId; 
  /** how many modulation parameters we have */
  unsigned char modulationParameterCount; 
  /** padding2 */
  unsigned short padding2; 
  /** padding3 */
  unsigned char padding3; 
  /** variable length list of modulation parameters */
  void * modulationParametersList; 
  /** variable length list of antenna pattern records */
  void * antennaPatternList; 
};

/* One track/jam target */
struct TrackJamTarget{
  /** track/jam target */
  struct EntityID trackJam; 
  /** Emitter ID */
  unsigned char emitterID; 
  /** beam ID */
  unsigned char beamID; 
};

/* Section 5.3.7.1. Information about active electronic warfare (EW) emissions and active EW countermeasures shall be communicated using an Electromagnetic Emission PDU. COMPLETE (I think) */
struct ElectronicEmissionsPdu{
  struct DistributedEmissionsFamilyPdu myDistributedEmissionsFamilyPdu;
  /** ID of the entity emitting */
  struct EntityID emittingEntityID; 
  /** ID of event */
  struct EventID eventID; 
  /** This field shall be used to indicate if the data in the PDU represents a state update or just data that has changed since issuance of the last Electromagnetic Emission PDU [relative to the identified entity and emission system(s)]. */
  unsigned char stateUpdateIndicator; 
  /** This field shall specify the number of emission systems being described in the current PDU. */
  unsigned char numberOfSystems; 
  /** padding */
  unsigned short paddingForEmissionsPdu; 
  /** Electronic emmissions systems */
  void * systems; 
};

/* Section 5.3.5.2. Information about a request for supplies. COMPLETE */
struct ResupplyOfferPdu{
  struct LogisticsFamilyPdu myLogisticsFamilyPdu;
  /** Entity that is receiving service */
  struct EntityID receivingEntityID; 
  /** Entity that is supplying */
  struct EntityID supplyingEntityID; 
  /** how many supplies are being offered */
  unsigned char numberOfSupplyTypes; 
  /** padding */
  short padding1; 
  /** padding */
  char padding2; 
  void * supplies; 
};

/* Section 5.3.10.1 Abstract superclass for PDUs relating to minefields */
struct MinefieldFamilyPdu{
  struct Pdu myPdu;
};

/* Section 5.3.12.9: initializing or chaning internal state information, reliable. Needs manual intervention to fix     padding on variable datums. UNFINISHED */
struct SetDataReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** Request ID */
  unsigned int requestID; 
  /** Fixed datum record count */
  unsigned int numberOfFixedDatumRecords; 
  /** variable datum record count */
  unsigned int numberOfVariableDatumRecords; 
  /** Fixed datum records */
  void * fixedDatumRecords; 
  /** Variable datum records */
  void * variableDatumRecords; 
};

/* Section 5.3.6.11. Reports occurance of a significant event to the simulation manager. COMPLETE */
struct EventReportPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** Type of event */
  unsigned int eventType; 
  /** padding */
  unsigned int padding1; 
  /** Number of fixed datum records */
  unsigned int numberOfFixedDatumRecords; 
  /** Number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variable length list of fixed datums */
  void * fixedDatums; 
  /** variable length list of variable length datums */
  void * variableDatums; 
};

/* Section 5.3.11.3: Inormation abut the addition or modification of a synthecic enviroment object that is anchored      to the terrain with a single point. COMPLETE */
struct PointObjectStatePdu{
  struct SyntheticEnvironmentFamilyPdu mySyntheticEnvironmentFamilyPdu;
  /** Object in synthetic environment */
  struct EntityID objectID; 
  /** Object with which this point object is associated */
  struct EntityID referencedObjectID; 
  /** unique update number of each state transition of an object */
  unsigned short updateNumber; 
  /** force ID */
  unsigned char forceID; 
  /** modifications */
  unsigned char modifications; 
  /** Object type */
  struct ObjectType objectType; 
  /** Object location */
  struct Vector3Double objectLocation; 
  /** Object orientation */
  struct Orientation objectOrientation; 
  /** Object apperance */
  double objectAppearance; 
  /** requesterID */
  struct SimulationAddress requesterID; 
  /** receiver ID */
  struct SimulationAddress receivingID; 
  /** padding */
  unsigned int pad2; 
};

/* Section 5.3.11.1: Information about environmental effects and processes. This requires manual cleanup. the environmental        record is variable, as is the padding. UNFINISHED */
struct EnvironmentalProcessPdu{
  struct SyntheticEnvironmentFamilyPdu mySyntheticEnvironmentFamilyPdu;
  /** Environmental process ID */
  struct EntityID environementalProcessID; 
  /** Environment type */
  struct EntityType environmentType; 
  /** model type */
  unsigned char modelType; 
  /** Environment status */
  unsigned char environmentStatus; 
  /** number of environment records  */
  unsigned char numberOfEnvironmentRecords; 
  /** PDU sequence number for the environmentla process if pdu sequencing required */
  unsigned short sequenceNumber; 
  /** environemt records */
  void * environmentRecords; 
};

/* Section 5.3.6.10. Information issued in response to a data query pdu or a set data pdu is communicated using a data pdu. COMPLETE */
struct DataPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** ID of request */
  unsigned int requestID; 
  /** padding */
  unsigned int padding1; 
  /** Number of fixed datum records */
  unsigned int numberOfFixedDatumRecords; 
  /** Number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variable length list of fixed datums */
  void * fixedDatums; 
  /** variable length list of variable length datums */
  void * variableDatums; 
};

/* Section 5.3.9.2 Information about a particular group of entities grouped together for the purposes of netowrk bandwidth         reduction or aggregation. Needs manual cleanup. The GED size requires a database lookup. UNFINISHED */
struct IsGroupOfPdu{
  struct EntityManagementFamilyPdu myEntityManagementFamilyPdu;
  /** ID of aggregated entities */
  struct EntityID groupEntityID; 
  /** type of entities constituting the group */
  unsigned char groupedEntityCategory; 
  /** Number of individual entities constituting the group */
  unsigned char numberOfGroupedEntities; 
  /** padding */
  unsigned int pad2; 
  /** latitude */
  double latitude; 
  /** longitude */
  double longitude; 
  /** GED records about each individual entity in the group. ^^^this is wrong--need a database lookup to find the actual size of the list elements */
  void * groupedEntityDescriptions; 
};

/* Section 5.3.10.3 Information about individual mines within a minefield. This is very, very wrong. UNFINISHED */
struct MinefieldDataPdu{
  struct MinefieldFamilyPdu myMinefieldFamilyPdu;
  /** Minefield ID */
  struct EntityID minefieldID; 
  /** ID of entity making request */
  struct EntityID requestingEntityID; 
  /** Minefield sequence number */
  unsigned short minefieldSequenceNumbeer; 
  /** request ID */
  unsigned char requestID; 
  /** pdu sequence number */
  unsigned char pduSequenceNumber; 
  /** number of pdus in response */
  unsigned char numberOfPdus; 
  /** how many mines are in this PDU */
  unsigned char numberOfMinesInThisPdu; 
  /** how many sensor type are in this PDU */
  unsigned char numberOfSensorTypes; 
  /** padding */
  unsigned char pad2; 
  /** 32 boolean fields */
  unsigned int dataFilter; 
  /** Mine type */
  struct EntityType mineType; 
  /** Sensor types, each 16 bits long */
  void * sensorTypes; 
  /** Padding to get things 32-bit aligned. ^^^this is wrong--dyanmically sized padding needed */
  unsigned char pad3; 
  /** Mine locations */
  void * mineLocation; 
};

/* Section 5.3.9.3 Information initiating the dyanic allocation and control of simulation entities         between two simulation applications. Requires manual cleanup. The padding between record sets is variable. UNFINISHED */
struct TransferControlRequestPdu{
  struct EntityManagementFamilyPdu myEntityManagementFamilyPdu;
  /** ID of entity originating request */
  struct EntityID orginatingEntityID; 
  /** ID of entity receiving request */
  struct EntityID recevingEntityID; 
  /** ID ofrequest */
  unsigned int requestID; 
  /** required level of reliabliity service. */
  unsigned char requiredReliabilityService; 
  /** type of transfer desired */
  unsigned char tranferType; 
  /** The entity for which control is being requested to transfer */
  struct EntityID transferEntityID; 
  /** number of record sets to transfer */
  unsigned char numberOfRecordSets; 
  /** ^^^This is wrong--the RecordSet class needs more work */
  void * recordSets; 
};

/* Section 5.3.3. Common superclass for EntityState, Collision, collision-elastic, and entity state update PDUs. This should be abstract. COMPLETE */
struct EntityInformationFamilyPdu{
  struct Pdu myPdu;
};

/* Section 5.3.12.5: Ack receipt of a start-resume, stop-freeze, create-entity or remove enitty (reliable) pdus. COMPLETE */
struct AcknowledgeReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** ack flags */
  unsigned short acknowledgeFlag; 
  /** response flags */
  unsigned short responseFlag; 
  /** Request ID */
  unsigned int requestID; 
};

/* Section 5.3.12.3: Start resume simulation, relaible. COMPLETE */
struct StartResumeReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** time in real world for this operation to happen */
  struct ClockTime realWorldTime; 
  /** time in simulation for the simulation to resume */
  struct ClockTime simulationTime; 
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** Request ID */
  unsigned int requestID; 
};

/* Section 5.3.7.4.2 When present, layer 2 should follow layer 1 and have the following fields. This requires manual cleanup.        the beamData attribute semantics are used in multiple ways. UNFINSISHED */
struct IffAtcNavAidsLayer2Pdu{
  struct IffAtcNavAidsLayer1Pdu myIffAtcNavAidsLayer1Pdu;
  /** layer header */
  struct LayerHeader layerHeader; 
  /** beam data */
  struct BeamData beamData; 
  /** Secondary operational data, 5.2.57 */
  struct BeamData secondaryOperationalData; 
  /** variable length list of fundamental parameters. ^^^This is wrong */
  void * fundamentalIffParameters; 
};

/* Section 5.3.11.5: Information about the addition/modification of an oobject that is geometrically      achored to the terrain with a set of three or more points that come to a closure. COMPLETE */
struct ArealObjectStatePdu{
  struct SyntheticEnvironmentFamilyPdu mySyntheticEnvironmentFamilyPdu;
  /** Object in synthetic environment */
  struct EntityID objectID; 
  /** Object with which this point object is associated */
  struct EntityID referencedObjectID; 
  /** unique update number of each state transition of an object */
  unsigned short updateNumber; 
  /** force ID */
  unsigned char forceID; 
  /** modifications enumeration */
  unsigned char modifications; 
  /** Object type */
  struct EntityType objectType; 
  /** Object appearance */
  struct SixByteChunk objectAppearance; 
  /** Number of points */
  unsigned short numberOfPoints; 
  /** requesterID */
  struct SimulationAddress requesterID; 
  /** receiver ID */
  struct SimulationAddress receivingID; 
  /** location of object */
  void * objectLocation; 
};

/* Section 5.3.12.8: request for data from an entity. COMPLETE */
struct DataQueryReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** request ID */
  unsigned int requestID; 
  /** time interval between issuing data query PDUs */
  unsigned int timeInterval; 
  /** Fixed datum record count */
  unsigned int numberOfFixedDatumRecords; 
  /** variable datum record count */
  unsigned int numberOfVariableDatumRecords; 
  /** Fixed datum records */
  void * fixedDatumRecords; 
  /** Variable datum records */
  void * variableDatumRecords; 
};

/* Section 5.3.9.1 informationa bout aggregating entities anc communicating information about the aggregated entities.        requires manual intervention to fix the padding between entityID lists and silent aggregate sysem lists--this padding        is dependent on how many entityIDs there are, and needs to be on a 32 bit word boundary. UNFINISHED */
struct AggregateStatePdu{
  struct EntityManagementFamilyPdu myEntityManagementFamilyPdu;
  /** ID of aggregated entities */
  struct EntityID aggregateID; 
  /** force ID */
  unsigned char forceID; 
  /** state of aggregate */
  unsigned char aggregateState; 
  /** entity type of the aggregated entities */
  struct EntityType aggregateType; 
  /** formation of aggregated entities */
  unsigned int formation; 
  /** marking for aggregate; first char is charset type, rest is char data */
  struct AggregateMarking aggregateMarking; 
  /** dimensions of bounding box for the aggregated entities, origin at the center of mass */
  struct Vector3Float dimensions; 
  /** orientation of the bounding box */
  struct Orientation orientation; 
  /** center of mass of the aggregation */
  struct Vector3Double centerOfMass; 
  /** velocity of aggregation */
  struct Vector3Float velocity; 
  /** number of aggregates */
  unsigned short numberOfDisAggregates; 
  /** number of entities */
  unsigned short numberOfDisEntities; 
  /** number of silent aggregate types */
  unsigned short numberOfSilentAggregateTypes; 
  /** number of silent entity types */
  unsigned short numberOfSilentEntityTypes; 
  /** aggregates  list */
  void * aggregateIDList; 
  /** entity ID list */
  void * entityIDList; 
  /** ^^^padding to put the start of the next list on a 32 bit boundary. This needs to be fixed */
  unsigned char pad2; 
  /** silent entity types */
  void * silentAggregateSystemList; 
  /** silent entity types */
  void * silentEntitySystemList; 
  /** number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variableDatums */
  void * variableDatumList; 
};

/* 5.3.3.4. Nonstatic information about a particular entity may be communicated by issuing an Entity State Update PDU. COMPLETE */
struct EntityStateUpdatePdu{
  struct EntityInformationFamilyPdu myEntityInformationFamilyPdu;
  /** This field shall identify the entity issuing the PDU */
  struct EntityID entityID; 
  /** Padding */
  char padding1; 
  /** How many articulation parameters are in the variable length list */
  unsigned char numberOfArticulationParameters; 
  /** Describes the speed of the entity in the world */
  struct Vector3Float entityLinearVelocity; 
  /** describes the location of the entity in the world */
  struct Vector3Double entityLocation; 
  /** describes the orientation of the entity, in euler angles */
  struct Orientation entityOrientation; 
  /** a series of bit flags that are used to help draw the entity, such as smoking, on fire, etc. */
  int entityAppearance; 
  void * articulationParameters; 
};

/* Section 5.3.10.1 Abstract superclass for PDUs relating to minefields. COMPLETE */
struct MinefieldStatePdu{
  struct MinefieldFamilyPdu myMinefieldFamilyPdu;
  /** Minefield ID */
  struct EntityID minefieldID; 
  /** Minefield sequence */
  unsigned short minefieldSequence; 
  /** force ID */
  unsigned char forceID; 
  /** Number of permieter points */
  unsigned char numberOfPerimeterPoints; 
  /** type of minefield */
  struct EntityType minefieldType; 
  /** how many mine types */
  unsigned short numberOfMineTypes; 
  /** location of minefield in world coords */
  struct Vector3Double minefieldLocation; 
  /** orientation of minefield */
  struct Orientation minefieldOrientation; 
  /** appearance bitflags */
  unsigned short appearance; 
  /** protocolMode */
  unsigned short protocolMode; 
  /** perimeter points for the minefield */
  void * perimeterPoints; 
  /** Type of mines */
  void * mineType; 
};

/* Section 5.3.12.10: issued in response to a data query R or set dataR pdu. Needs manual intervention      to fix padding on variable datums. UNFINSIHED */
struct DataReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** Request ID */
  unsigned int requestID; 
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** Fixed datum record count */
  unsigned int numberOfFixedDatumRecords; 
  /** variable datum record count */
  unsigned int numberOfVariableDatumRecords; 
  /** Fixed datum records */
  void * fixedDatumRecords; 
  /** Variable datum records */
  void * variableDatumRecords; 
};

/* Section 5.3.6.12. Arbitrary messages can be entered into the data stream via use of this PDU. COMPLETE */
struct CommentPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** Number of fixed datum records */
  unsigned int numberOfFixedDatumRecords; 
  /** Number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variable length list of fixed datums */
  void * fixedDatums; 
  /** variable length list of variable length datums */
  void * variableDatums; 
};

/* Section 5.3.12.12: Arbitrary messages. Only reliable this time. Neds manual intervention     to fix padding in variable datums. UNFINISHED */
struct CommentReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** Fixed datum record count */
  unsigned int numberOfFixedDatumRecords; 
  /** variable datum record count */
  unsigned int numberOfVariableDatumRecords; 
  /** Fixed datum records */
  void * fixedDatumRecords; 
  /** Variable datum records */
  void * variableDatumRecords; 
};

/* Section 5.3.4.2. Information about stuff exploding. COMPLETE */
struct DetonationPdu{
  struct WarfareFamilyPdu myWarfareFamilyPdu;
  /** ID of muntion that was fired */
  struct EntityID munitionID; 
  /** ID firing event */
  struct EventID eventID; 
  /** ID firing event */
  struct Vector3Float velocity; 
  /** where the detonation is, in world coordinates */
  struct Vector3Double locationInWorldCoordinates; 
  /** Describes munition used */
  struct BurstDescriptor burstDescriptor; 
  /** location of the detonation or impact in the target entity's coordinate system. This information should be used for damage assessment. */
  struct Vector3Float locationInEntityCoordinates; 
  /** result of the explosion */
  unsigned char detonationResult; 
  /** How many articulation parameters we have */
  unsigned char numberOfArticulationParameters; 
  /** padding */
  short pad; 
  void * articulationParameters; 
};

/* Section 5.3.6.9. Change state information with the data contained in this. COMPLETE */
struct SetDataPdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** ID of request */
  unsigned int requestID; 
  /** padding */
  unsigned int padding1; 
  /** Number of fixed datum records */
  unsigned int numberOfFixedDatumRecords; 
  /** Number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variable length list of fixed datums */
  void * fixedDatums; 
  /** variable length list of variable length datums */
  void * variableDatums; 
};

/* Section 5.3.12.13: A request for one or more records of data from an entity. COMPLETE */
struct RecordQueryReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** request ID */
  unsigned int requestID; 
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding. The spec is unclear and contradictory here. */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** event type */
  unsigned short eventType; 
  /** time */
  unsigned int time; 
  /** numberOfRecords */
  unsigned int numberOfRecords; 
  /** record IDs */
  void * recordIDs; 
};

/* Section 5.3.3.2. Information about a collision. COMPLETE */
struct CollisionPdu{
  struct EntityInformationFamilyPdu myEntityInformationFamilyPdu;
  /** ID of the entity that issued the collision PDU */
  struct EntityID issuingEntityID; 
  /** ID of entity that has collided with the issuing entity ID */
  struct EntityID collidingEntityID; 
  /** ID of event */
  struct EventID eventID; 
  /** ID of event */
  unsigned char collisionType; 
  /** some padding */
  char pad; 
  /** velocity at collision */
  struct Vector3Float velocity; 
  /** mass of issuing entity */
  float mass; 
  /** Location with respect to entity the issuing entity collided with */
  struct Vector3Float location; 
};

/* Section 5.3.6.7. response to an action request PDU. COMPLETE */
struct ActionResponsePdu{
  struct SimulationManagementFamilyPdu mySimulationManagementFamilyPdu;
  /** Request ID that is unique */
  unsigned int requestID; 
  /** Status of response */
  unsigned int requestStatus; 
  /** Number of fixed datum records */
  unsigned int numberOfFixedDatumRecords; 
  /** Number of variable datum records */
  unsigned int numberOfVariableDatumRecords; 
  /** variable length list of fixed datums */
  void * fixedDatums; 
  /** variable length list of variable length datums */
  void * variableDatums; 
};

/* Sectioin 5.3.4.1. Information about someone firing something. COMPLETE */
struct FirePdu{
  struct WarfareFamilyPdu myWarfareFamilyPdu;
  /** ID of the munition that is being shot */
  struct EntityID munitionID; 
  /** ID of event */
  struct EventID eventID; 
  int fireMissionIndex; 
  /** location of the firing event */
  struct Vector3Double locationInWorldCoordinates; 
  /** Describes munitions used in the firing event */
  struct BurstDescriptor burstDescriptor; 
  /** Velocity of the ammunition */
  struct Vector3Float velocity; 
  /** range to the target */
  float range; 
};

/* Section 5.3.8.3. Communication of a receiver state. COMPLETE */
struct ReceiverPdu{
  struct RadioCommunicationsFamilyPdu myRadioCommunicationsFamilyPdu;
  /** encoding scheme used, and enumeration */
  unsigned short receiverState; 
  /** padding */
  unsigned short padding1; 
  /** received power */
  float receivedPoser; 
  /** ID of transmitter */
  struct EntityID transmitterEntityId; 
  /** ID of transmitting radio */
  unsigned short transmitterRadioId; 
};

/* Section 5.3.7.3. Information about underwater acoustic emmissions. This requires manual cleanup.  The beam data records should ALL be a the finish, rather than attached to each emitter system. UNFINISHED */
struct UaPdu{
  struct DistributedEmissionsFamilyPdu myDistributedEmissionsFamilyPdu;
  /** ID of the entity that is the source of the emission */
  struct EntityID emittingEntityID; 
  /** ID of event */
  struct EventID eventID; 
  /** This field shall be used to indicate whether the data in the UA PDU represent a state update or data that have changed since issuance of the last UA PDU */
  char stateChangeIndicator; 
  /** padding */
  char pad; 
  /** This field indicates which database record (or file) shall be used in the definition of passive signature (unintentional) emissions of the entity. The indicated database record (or  file) shall define all noise generated as a function of propulsion plant configurations and associated  auxiliaries. */
  unsigned short passiveParameterIndex; 
  /** This field shall specify the entity propulsion plant configuration. This field is used to determine the passive signature characteristics of an entity. */
  unsigned char propulsionPlantConfiguration; 
  /**  This field shall represent the number of shafts on a platform */
  unsigned char numberOfShafts; 
  /** This field shall indicate the number of APAs described in the current UA PDU */
  unsigned char numberOfAPAs; 
  /** This field shall specify the number of UA emitter systems being described in the current UA PDU */
  unsigned char numberOfUAEmitterSystems; 
  /** shaft RPM values */
  void * shaftRPMs; 
  /** apaData */
  void * apaData; 
  void * emitterSystems; 
};

/* Section 5.3.8.5. Detailed inofrmation about the state of an intercom device and the actions it is requestion         of another intercom device, or the response to a requested action. Required manual intervention to fix the intercom parameters,        which can be of varialbe length. UNFINSISHED */
struct IntercomControlPdu{
  struct RadioCommunicationsFamilyPdu myRadioCommunicationsFamilyPdu;
  /** control type */
  unsigned char controlType; 
  /** control type */
  unsigned char communicationsChannelType; 
  /** Source entity ID */
  struct EntityID sourceEntityID; 
  /** The specific intercom device being simulated within an entity. */
  unsigned char sourceCommunicationsDeviceID; 
  /** Line number to which the intercom control refers */
  unsigned char sourceLineID; 
  /** priority of this message relative to transmissons from other intercom devices */
  unsigned char transmitPriority; 
  /** current transmit state of the line */
  unsigned char transmitLineState; 
  /** detailed type requested. */
  unsigned char command; 
  /** eid of the entity that has created this intercom channel. */
  struct EntityID masterEntityID; 
  /** specific intercom device that has created this intercom channel */
  unsigned short masterCommunicationsDeviceID; 
  /** number of intercom parameters */
  unsigned int intercomParametersLength; 
  /** ^^^This is wrong--the length of the data field is variable. Using a long for now. */
  void * intercomParameters; 
};

/* Section 5.3.8.2. Detailed information about a radio transmitter. This PDU requires        manually written code to complete. The encodingScheme field can be used in multiple        ways, which requires hand-written code to finish. UNFINISHED */
struct SignalPdu{
  struct RadioCommunicationsFamilyPdu myRadioCommunicationsFamilyPdu;
  /** encoding scheme used, and enumeration */
  unsigned short encodingScheme; 
  /** tdl type */
  unsigned short tdlType; 
  /** sample rate */
  unsigned int sampleRate; 
  /** length od data */
  short dataLength; 
  /** number of samples */
  short samples; 
  /** list of eight bit values */
  void * data; 
};

/* Section 5.3.12.2: Removal of an entity , reliable. COMPLETE */
struct RemoveEntityReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** Request ID */
  unsigned int requestID; 
};

/* Section 5.3.7.5. SEES PDU, supplemental emissions entity state information. COMPLETE */
struct SeesPdu{
  struct DistributedEmissionsFamilyPdu myDistributedEmissionsFamilyPdu;
  /** Originating entity ID */
  struct EntityID orginatingEntityID; 
  /** IR Signature representation index */
  unsigned short infraredSignatureRepresentationIndex; 
  /** acoustic Signature representation index */
  unsigned short acousticSignatureRepresentationIndex; 
  /** radar cross section representation index */
  unsigned short radarCrossSectionSignatureRepresentationIndex; 
  /** how many propulsion systems */
  unsigned short numberOfPropulsionSystems; 
  /** how many vectoring nozzle systems */
  unsigned short numberOfVectoringNozzleSystems; 
  /** variable length list of propulsion system data */
  void * propulsionSystemData; 
  /** variable length list of vectoring system data */
  void * vectoringSystemData; 
};

/* Section 5.3.12.1: creation of an entity , reliable. COMPLETE */
struct CreateEntityReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** level of reliability service used for this transaction */
  unsigned char requiredReliabilityService; 
  /** padding */
  unsigned short pad1; 
  /** padding */
  unsigned char pad2; 
  /** Request ID */
  unsigned int requestID; 
};

/* Section 5.3.12.4: Stop freeze simulation, relaible. COMPLETE */
struct StopFreezeReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** time in real world for this operation to happen */
  struct ClockTime realWorldTime; 
  /** Reason for stopping/freezing simulation */
  unsigned char reason; 
  /** internal behvior of the simulation while frozen */
  unsigned char frozenBehavior; 
  /** reliablity level */
  unsigned char requiredReliablityService; 
  /** padding */
  unsigned char pad1; 
  /** Request ID */
  unsigned int requestID; 
};

/* Section 5.3.12.11: reports the occurance of a significatnt event to the simulation manager. Needs manual     intervention to fix padding in variable datums. UNFINISHED. */
struct EventReportReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** Event type */
  unsigned short eventType; 
  /** padding */
  unsigned int pad1; 
  /** Fixed datum record count */
  unsigned int numberOfFixedDatumRecords; 
  /** variable datum record count */
  unsigned int numberOfVariableDatumRecords; 
  /** Fixed datum records */
  void * fixedDatumRecords; 
  /** Variable datum records */
  void * variableDatumRecords; 
};

/* Section 5.3.10.4 proivde the means to request a retransmit of a minefield data pdu. COMPLETE */
struct MinefieldResponseNackPdu{
  struct MinefieldFamilyPdu myMinefieldFamilyPdu;
  /** Minefield ID */
  struct EntityID minefieldID; 
  /** entity ID making the request */
  struct EntityID requestingEntityID; 
  /** request ID */
  unsigned char requestID; 
  /** how many pdus were missing */
  unsigned char numberOfMissingPdus; 
  /** PDU sequence numbers that were missing */
  void * missingPduSequenceNumbers; 
};

/* 5.3.3.3. Information about elastic collisions in a DIS exercise shall be communicated using a Collision-Elastic PDU. COMPLETE */
struct CollisionElasticPdu{
  struct EntityInformationFamilyPdu myEntityInformationFamilyPdu;
  /** ID of the entity that issued the collision PDU */
  struct EntityID issuingEntityID; 
  /** ID of entity that has collided with the issuing entity ID */
  struct EntityID collidingEntityID; 
  /** ID of event */
  struct EventID collisionEventID; 
  /** some padding */
  short pad; 
  /** velocity at collision */
  struct Vector3Float contactVelocity; 
  /** mass of issuing entity */
  float mass; 
  /** Location with respect to entity the issuing entity collided with */
  struct Vector3Float location; 
  /** tensor values */
  float collisionResultXX; 
  /** tensor values */
  float collisionResultXY; 
  /** tensor values */
  float collisionResultXZ; 
  /** tensor values */
  float collisionResultYY; 
  /** tensor values */
  float collisionResultYZ; 
  /** tensor values */
  float collisionResultZZ; 
  /** This record shall represent the normal vector to the surface at the point of collision detection. The surface normal shall be represented in world coordinates. */
  struct Vector3Float unitSurfaceNormal; 
  /** This field shall represent the degree to which energy is conserved in a collision */
  float coefficientOfRestitution; 
};

/* Section 5.3.12.7: Response from an entity to an action request PDU. COMPLETE */
struct ActionResponseReliablePdu{
  struct SimulationManagementWithReliabilityFamilyPdu mySimulationManagementWithReliabilityFamilyPdu;
  /** request ID */
  unsigned int requestID; 
  /** status of response */
  unsigned int responseStatus; 
  /** Fixed datum record count */
  unsigned int numberOfFixedDatumRecords; 
  /** variable datum record count */
  unsigned int numberOfVariableDatumRecords; 
  /** Fixed datum records */
  void * fixedDatumRecords; 
  /** Variable datum records */
  void * variableDatumRecords; 
};

/* Section 5.3.9.4 The joining of two or more simulation entities is communicated by this PDU. COMPLETE */
struct IsPartOfPdu{
  struct EntityManagementFamilyPdu myEntityManagementFamilyPdu;
  /** ID of entity originating PDU */
  struct EntityID orginatingEntityID; 
  /** ID of entity receiving PDU */
  struct EntityID receivingEntityID; 
  /** relationship of joined parts */
  struct Relationship relationship; 
  /** location of part; centroid of part in host's coordinate system. x=range, y=bearing, z=0 */
  struct Vector3Float partLocation; 
  /** named location */
  struct NamedLocation namedLocationID; 
  /** entity type */
  struct EntityType partEntityType; 
};

/* Section 5.3.10.2 Query a minefield for information about individual mines. Requires manual clean up to get the padding right. UNFINISHED */
struct MinefieldQueryPdu{
  struct MinefieldFamilyPdu myMinefieldFamilyPdu;
  /** Minefield ID */
  struct EntityID minefieldID; 
  /** EID of entity making the request */
  struct EntityID requestingEntityID; 
  /** request ID */
  unsigned char requestID; 
  /** Number of perimeter points for the minefield */
  unsigned char numberOfPerimeterPoints; 
  /** Padding */
  unsigned char pad2; 
  /** Number of sensor types */
  unsigned char numberOfSensorTypes; 
  /** data filter, 32 boolean fields */
  unsigned int dataFilter; 
  /** Entity type of mine being requested */
  struct EntityType requestedMineType; 
  /** perimeter points of request */
  void * requestedPerimeterPoints; 
  /** Sensor types, each 16 bits long */
  void * sensorTypes; 
};

/* Section 5.3.3.1. Represents the postion and state of one entity in the world. COMPLETE */
struct EntityStatePdu{
  struct EntityInformationFamilyPdu myEntityInformationFamilyPdu;
  /** Unique ID for an entity that is tied to this state information */
  struct EntityID entityID; 
  /** What force this entity is affiliated with, eg red, blue, neutral, etc */
  unsigned char forceId; 
  /** How many articulation parameters are in the variable length list */
  char numberOfArticulationParameters; 
  /** Describes the type of entity in the world */
  struct EntityType entityType; 
  struct EntityType alternativeEntityType; 
  /** Describes the speed of the entity in the world */
  struct Vector3Float entityLinearVelocity; 
  /** describes the location of the entity in the world */
  struct Vector3Double entityLocation; 
  /** describes the orientation of the entity, in euler angles */
  struct Orientation entityOrientation; 
  /** a series of bit flags that are used to help draw the entity, such as smoking, on fire, etc. */
  int entityAppearance; 
  /** parameters used for dead reckoning */
  struct DeadReckoningParameter deadReckoningParameters; 
  /** characters that can be used for debugging, or to draw unique strings on the side of entities in the world */
  struct Marking marking; 
  /** a series of bit flags */
  int capabilities; 
  /** variable length list of articulation parameters */
  void * articulationParameters; 
};

/* Section 5.3.3.1. Represents the postion and state of one entity in the world. This is identical in function to entity state pdu, but generates less garbage to collect in the Java world. COMPLETE */
struct FastEntityStatePdu{
  struct EntityInformationFamilyPdu myEntityInformationFamilyPdu;
  /** The site ID */
  unsigned short site; 
  /** The application ID */
  unsigned short application; 
  /** the entity ID */
  unsigned short entity; 
  /** what force this entity is affiliated with, eg red, blue, neutral, etc */
  unsigned char forceId; 
  /** How many articulation parameters are in the variable length list */
  char numberOfArticulationParameters; 
  /** Kind of entity */
  unsigned char entityKind; 
  /** Domain of entity (air, surface, subsurface, space, etc) */
  unsigned char domain; 
  /** country to which the design of the entity is attributed */
  unsigned short country; 
  /** category of entity */
  unsigned char category; 
  /** subcategory of entity */
  unsigned char subcategory; 
  /** specific info based on subcategory field */
  unsigned char specific; 
  unsigned char extra; 
  /** Kind of entity */
  unsigned char altEntityKind; 
  /** Domain of entity (air, surface, subsurface, space, etc) */
  unsigned char altDomain; 
  /** country to which the design of the entity is attributed */
  unsigned short altCountry; 
  /** category of entity */
  unsigned char altCategory; 
  /** subcategory of entity */
  unsigned char altSubcategory; 
  /** specific info based on subcategory field */
  unsigned char altSpecific; 
  unsigned char altExtra; 
  /** X velo */
  float xVelocity; 
  /** y Value */
  float yVelocity; 
  /** Z value */
  float zVelocity; 
  /** X value */
  double xLocation; 
  /** y Value */
  double yLocation; 
  /** Z value */
  double zLocation; 
  float psi; 
  float theta; 
  float phi; 
  /** a series of bit flags that are used to help draw the entity, such as smoking, on fire, etc. */
  int entityAppearance; 
  /** enumeration of what dead reckoning algorighm to use */
  unsigned char deadReckoningAlgorithm; 
  /** other parameters to use in the dead reckoning algorithm */
  char otherParameters[15]; 
  /** X value */
  float xAcceleration; 
  /** y Value */
  float yAcceleration; 
  /** Z value */
  float zAcceleration; 
  /** X value */
  float xAngularVelocity; 
  /** y Value */
  float yAngularVelocity; 
  /** Z value */
  float zAngularVelocity; 
  /** characters that can be used for debugging, or to draw unique strings on the side of entities in the world */
  char marking[12]; 
  /** a series of bit flags */
  int capabilities; 
  /** variable length list of articulation parameters */
  void * articulationParameters; 
};


unsigned char * dis_ctor(int distype);
void dis_dtor(unsigned char *item, int distype);
unsigned char * dis_marshal(unsigned char * datastream, unsigned char *item, int type);
unsigned char *dis_unmarshal(unsigned char *datastream, unsigned char* item, int type);
int pduToDis(int pdu)
;
#ifdef __cplusplus
}
#endif

#endif //DIS_H

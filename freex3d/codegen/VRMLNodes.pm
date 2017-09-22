#
# Copyright (C) 1998 Tuomas J. Lukka 1999 John Stewart CRC Canada.
# DISTRIBUTED WITH NO WARRANTY, EXPRESS OR IMPLIED.
# See the GNU Library General Public License (file COPYING in the distribution)
# for conditions of use and redistribution.

package VRML::NodeType;

use strict;
use warnings;

# see note top of file - the VRML Parser REQUIRES for routing that
# each field name exists in only one table- eg, "inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)", "initializeOnly, (SPEC_VRML | SPEC_X3D30)",
# etc. So, in order to do this, we make sure that each field name follows this, even
# though we may, for instance, change "value" to an "inputOutput, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)" when the spec
# says it is an "initializeOnly, "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)". This has little if any effect on parsing.

# SPEC_VRML tag verified against http://web3d.org/x3d/specifications/vrml/ISO-IEC-14772-VRML97/part1/nodesRef.html

########################################################################

{
   sub new {
		my($type, $name, $fields, $X3DNodeType) = @_;
		if ($X3DNodeType eq "") {
			print "NodeType, X3DNodeType blank for $name\n";
			$X3DNodeType = "unknown";
		}
		# DEBUG: print "Node: $X3DNodeType\n";
		my $this = bless {
						  Name => $name,
						  Defaults => {},
						  X3DNodeType => $X3DNodeType
						 },$type;
		my $t;
		my $i;
		my $j;
		my $fname;
		my $farray;
		my @field;
		my $size = @$fields;
		my @fnames;
		#for (keys %$fields) {
		for($i=0; $i < $size; $i = $i + 2) {
			$j = $i + 1;
			$fname = $fields->[$i];
			push(@fnames, $fname);
			$farray = $fields->[$j];
			@field = @$farray;
			#print "field key $_\n";
			#print "fname $fname field @field $field[0]\n";
			if (ref $field[1] eq "ARRAY") {
				push @{$this->{Defaults}{$fname}}, @{$field[1]};
			} else {
				$this->{Defaults}{$fname} = $field[1];
			}
			$this->{FieldTypes}{$fname} = $field[0];

			$t = $field[2];
			if (!defined $t) {
				die("Missing field or event type $type X3DNodeType $X3DNodeType for $fname in $name");
			}
			$this->{FieldKinds}{$fname} = $t;

			$t = $field[3];
			if (!defined $t) {
				die("Missing field or event type $type X3DNodeType $X3DNodeType for $fname in $name");
			}
			$this->{SpecLevel}{$fname} = $t;

		}
		$this->{fnames} = \@fnames;
		return $this;
    }
}

our %Nodes = (

	###################################################################################

	# chapter 7: 		Core Component

	###################################################################################


	"WorldInfo" => new VRML::NodeType("WorldInfo", [
		info => ["MFString", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		title => ["SFString", "", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"Proto" => new VRML::NodeType("Proto", [
		# sept 2014: keep Inline the same as Proto, so one can be cast to the other, unless/until executionContext is extracted from both
		__children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__protoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__externProtoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__nodes => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__subcontexts => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__GC => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__protoDef => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"], #user fields
		__protoFlags => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__prototype => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], #first node in protobody
		__parentProto => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], #first node in protobody
		__ROUTES => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__EXPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__IMPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__DEFnames => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__IS => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__scripts => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__oldurl => ["MFString", [], "initializeOnly", 0,"UNCA_NONE"],
		__afterPound => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__typename => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		load => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__oldload => ["SFBool", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__unitlengthfactor => ["SFDouble", 1.0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DProtoInstance"),

	"MetadataBoolean" => new VRML::NodeType("MetadataBoolean", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			value => ["MFBool",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],  # see note top of file
	], "X3DChildNode"),

	"MetadataInteger" => new VRML::NodeType("MetadataInteger", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			value => ["MFInt32",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],  # see note top of file
	], "X3DChildNode"),

	"MetadataDouble" => new VRML::NodeType("MetadataDouble", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],# see note top of file:
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			value => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
	], "X3DChildNode"),

	"MetadataFloat" => new VRML::NodeType("MetadataFloat", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			value => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
	], "X3DChildNode"),

	"MetadataString" => new VRML::NodeType("MetadataString", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			value => ["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
	], "X3DChildNode"),

	"MetadataSet" => new VRML::NodeType("MetadataSet", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
			value => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
	], "X3DChildNode"),

	###################################################################################

	# Chapter 8:		Time Component

	###################################################################################

	"TimeSensor" => new VRML::NodeType("TimeSensor", [
		cycleInterval => ["SFTime", 1, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		loop => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pauseTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		resumeTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		startTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		cycleTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		elapsedTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fraction_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isPaused => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		time => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# time that we were initialized at
		__inittime => ["SFTime", 0, "initializeOnly", 0,"UNCA_NONE"],
		# cycleTimer flag.
		__ctflag =>["SFTime", 10, "inputOutput", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
		__lasttime => ["SFTime", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DSensorNode"),

	###################################################################################

	# Chapter 9:		Networking Component

	###################################################################################

	"Anchor" => new VRML::NodeType("Anchor", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		parameter => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),


	"Inline" => new VRML::NodeType("Inline", [
		# sept 2014: keep Inline the same as Proto, so one can be cast to the other, unless/until executionContext is extracted from both
		__children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__protoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__externProtoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__nodes => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__subcontexts => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__GC => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__protoDef => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"], #user fields
		__protoFlags => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__prototype => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], #first node in protobody
		__parentProto => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], #first node in protobody
		__ROUTES => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__EXPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__IMPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__DEFnames => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__IS => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__scripts => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__oldurl => ["MFString", [], "initializeOnly", 0,"UNCA_NONE"],
		__afterPound => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__typename => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		load => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__oldload => ["SFBool", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__unitlengthfactor => ["SFDouble", 1.0, "initializeOnly", 0,"UNCA_NONE"],
		
		
		# load => ["SFBool", "TRUE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

                # __children => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		# __loadstatus =>["SFInt32",0,"initializeOnly", 0],
		# _parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		 # __loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DNetworkSensorNode"),

	"LoadSensor" => new VRML::NodeType("LoadSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		timeOut  => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		watchList => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isLoaded  => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		loadTime  => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		progress  => ["SFFloat",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		__loading => ["SFBool", "TRUE","initializeOnly", 0,"UNCA_NONE"],		# current internal status
		__finishedloading => ["SFBool", "TRUE","initializeOnly", 0,"UNCA_NONE"],	# current internal status
		__StartLoadTime => ["SFTime",0,"outputOnly", 0,"UNCA_NONE"], # time we started loading...
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DNetworkSensorNode"),


	###################################################################################

	# Chapter 10:		Grouping Component

	###################################################################################

	"Group" => new VRML::NodeType("Group", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),

	"StaticGroup" => new VRML::NodeType("StaticGroup", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		__transparency => ["SFInt32", -1, "initializeOnly", 0,"UNCA_NONE"], # display list for transparencies
		__solid => ["SFInt32", -1, "initializeOnly", 0,"UNCA_NONE"],	 # display list for solid geoms.
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),

	"Switch" => new VRML::NodeType("Switch", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		choice => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30)","UNCA_NONE"],		# VRML nodes....
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],		# X3D nodes....
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		whichChoice => ["SFInt32", -1, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		__isX3D => ["SFBool", "(inputFileVersion[0]==3)" , "initializeOnly", 0,"UNCA_NONE"], # "TRUE" for X3D V3.x files
	],"X3DGroupingNode"),

	"Transform" => new VRML::NodeType ("Transform", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# fields for reducing redundant calls
		__do_center => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_scaleO => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_scale => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_anything => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),


	###################################################################################

	# Chapter 11:		Rendering Component

	###################################################################################

	"ClipPlane" => new VRML::NodeType("ClipPlane", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		plane => ["SFVec4f", [0, 1, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

	],"X3DChildNode"),

	"Color" => new VRML::NodeType("Color", [
		color => ["MFColor", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

	],"X3DColorNode"),

	"ColorRGBA" => new VRML::NodeType("ColorRGBA", [
		color => ["MFColorRGBA", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

	],"X3DColorNode"),

	"Coordinate" => new VRML::NodeType("Coordinate", [
		point => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

	],"X3DCoordinateNode"),

	"IndexedLineSet" => new VRML::NodeType("IndexedLineSet", [
		set_colorIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_coordIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coordIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__vertArr  =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__vertIndx  =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__xcolours  =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__vertices  =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__vertexCount =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__segCount =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"IndexedTriangleFanSet" => new VRML::NodeType("IndexedTriangleFanSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		index => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"IndexedTriangleSet" => new VRML::NodeType("IndexedTriangleSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		index => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"IndexedTriangleStripSet" => new VRML::NodeType("IndexedTriangleStripSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		index => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"LineSet" => new VRML::NodeType("LineSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vertexCount => ["MFInt32",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__vertArr  =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__vertIndx  =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__segCount =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"Normal" => new VRML::NodeType("Normal", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vector => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DNormalNode"),

	"PointSet" => new VRML::NodeType("PointSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_pointsVBO =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_coloursVBO =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_npoints =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_colourSize =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"TriangleFanSet" => new VRML::NodeType("TriangleFanSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fanCount => ["MFInt32", [3], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"TriangleStripSet" => new VRML::NodeType("TriangleStripSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stripCount => ["MFInt32", [], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],

	],"X3DGeometryNode"),

	"TriangleSet" => new VRML::NodeType("TriangleSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),


	###################################################################################

#	Chapter 12:		Shape Component

	###################################################################################

	"Appearance" => new VRML::NodeType ("Appearance", [
		fillProperties => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		lineProperties => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		material => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		shaders => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		effects => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texture => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureTransform => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DAppearanceNode"),

	"FillProperties" => new VRML::NodeType ("FillProperties", [
		filled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hatchColor => ["SFColor", [1,1,1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hatched => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hatchStyle => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_enabled =>["SFBool", "TRUE", "inputOutput",0,"UNCA_NONE"], # literally, is this thing used or not?
		_hatchScale =>["SFVec2f", [0.1,0.1], "inputOutput",0,"UNCA_NONE"], # the rate of the lines, 0.1 = 10 lines/meter
	],"X3DAppearanceChildNode"),

	"LineProperties" => new VRML::NodeType ("LineProperties", [
		applied => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		linetype => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		linewidthScaleFactor => ["SFFloat", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DAppearanceChildNode"),

	"Material" => new VRML::NodeType ("Material", [
		ambientIntensity => ["SFFloat", 0.2, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		diffuseColor => ["SFColor", [0.8, 0.8, 0.8], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		emissiveColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		shininess => ["SFFloat", 0.2, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		specularColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transparency => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_verifiedColor => ["MFFloat",[
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0],"initializeOnly",0,"UNCA_NONE"], # for making materials shader-friendly
	],"X3DMaterialNode"),

	"Shape" => new VRML::NodeType ("Shape", [
		# shared with particlesystem, keep in same order as particlesystem:
		appearance => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_shaderflags_base =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"], # shaders
		_shaderflags_effects =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"], # shaders
		_shaderflags_usershaders =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"], # shaders
		# shape-specific:
		__visible =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"], # for Occlusion tests.
		__occludeCheckCount =>["SFInt32",-1,"initializeOnly", 0,"UNCA_NONE"], # for Occlusion tests.
		__Samples =>["SFInt32",-1,"initializeOnly", 0,"UNCA_NONE"],		# Occlude samples from last pass

	],"X3DBoundedObject"),

	"TwoSidedMaterial" => new VRML::NodeType ("TwoSidedMaterial", [
		ambientIntensity => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		backAmbientIntensity => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		backDiffuseColor => ["SFColor", [0.8, 0.8, 0.8], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		backEmissiveColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		backShininess => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		backSpecularColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		backTransparency => ["SFFloat", 0, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		diffuseColor => ["SFColor", [0.8, 0.8, 0.8], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		emissiveColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		shininess => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		separateBackColor =>["SFBool","FALSE","inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		specularColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		transparency => ["SFFloat", 0, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		_verifiedFrontColor => ["MFFloat",[
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0],"initializeOnly",0,"UNCA_NONE"], # for making materials shader-friendly
		_verifiedBackColor => ["MFFloat",[
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0],"initializeOnly",0,"UNCA_NONE"], # for making materials shader-friendly
	],"X3DMaterialNode"),



	###################################################################################

	# Chapter 13:		Geometry3D Component

	###################################################################################

	"Box" => new VRML::NodeType("Box", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFVec3f", [2, 2, 2], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__points  =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"Cone" => new VRML::NodeType ("Cone", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bottom => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		bottomRadius => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		height => ["SFFloat", 2.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		side => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		 __sidepoints =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		 __botpoints =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		 __normals =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		__coneVBO =>["SFInt32",0,"initializeOnly","UNCA_NONE"],
		__coneTriangles =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"Cylinder" => new VRML::NodeType ("Cylinder", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bottom => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		height => ["SFFloat", 2.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		side => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		top => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		 __points =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		 __normals =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		__cylinderVBO =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		__cylinderTriangles =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"ElevationGrid" => new VRML::NodeType("ElevationGrid", [
		set_height => ["MFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		creaseAngle => ["SFFloat", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		height => ["MFFloat", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		xDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		xSpacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		zDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		zSpacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"Extrusion" => new VRML::NodeType("Extrusion", [
		set_crossSection => ["MFVec2f", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_orientation => ["MFRotation", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_scale => ["MFVec2f", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_spine => ["MFVec3f", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		beginCap => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		convex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		creaseAngle => ["SFFloat", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		crossSection => ["MFVec2f", [[1, 1],[1, -1],[-1, -1],
						   [-1, 1],[1, 1]], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		endCap => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orientation => ["MFRotation", [[0, 0, 1, 0]],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		scale => ["MFVec2f", [[1, 1]], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		spine => ["MFVec3f", [[0, 0, 0],[0, 1, 0]], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DGeometryNode"),

	"IndexedFaceSet" => new VRML::NodeType("IndexedFaceSet", [
		set_colorIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_coordIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_normalIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_texCoordIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		convex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coordIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		creaseAngle => ["SFFloat", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoordIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DGeometryNode"),

	"Sphere" => new VRML::NodeType("Sphere", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__points =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		_sideVBO =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__SphereIndxVBO =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__pindices => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__wireindicesVBO =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
 	],"X3DGeometryNode"),

	"Teapot" => new VRML::NodeType("Teapot", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__ifsnode => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
 	],"X3DGeometryNode"),



	###################################################################################

	#	Chapter 14:	Geometry 2D Component

	###################################################################################

	"Arc2D" => new VRML::NodeType("Arc2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		endAngle => ["SFFloat", 1.5707, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		startAngle => ["SFFloat", 0.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__points  =>["MFVec2f",[],"initializeOnly", 0,"UNCA_NONE"],
		__numPoints =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
 	],"X3DGeometryNode"),

	"ArcClose2D" => new VRML::NodeType("ArcClose2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		closureType => ["SFString","PIE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		endAngle => ["SFFloat", 1.5707, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		startAngle => ["SFFloat", 0.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__points  =>["MFVec2f",[],"initializeOnly", 0,"UNCA_NONE"],
		__numPoints =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
 	],"X3DGeometryNode"),


	"Circle2D" => new VRML::NodeType("Circle2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		__points  =>["MFVec2f",[],"initializeOnly", 0,"UNCA_NONE"],
		__numPoints =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
 	],"X3DGeometryNode"),

	"Disk2D" => new VRML::NodeType("Disk2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		innerRadius => ["SFFloat", 0.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		outerRadius => ["SFFloat", 1.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__points  =>["MFVec2f",[],"initializeOnly", 0,"UNCA_NONE"],
		__texCoords  =>["MFVec2f",[],"initializeOnly", 0,"UNCA_NONE"],
		__numPoints =>["SFInt32",0,"initializeOnly", 0],
		__simpleDisk => ["SFBool", "TRUE","initializeOnly", 0,"UNCA_NONE"],
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"Polyline2D" => new VRML::NodeType("Polyline2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		lineSegments => ["MFVec2f", [], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
 	],"X3DGeometryNode"),

	"Polypoint2D" => new VRML::NodeType("Polypoint2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		point => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
 	],"X3DGeometryNode"),

	"Rectangle2D" => new VRML::NodeType("Rectangle2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFVec2f", [2.0, 2.0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__points  =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		__numPoints =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
 	],"X3DGeometryNode"),


	"TriangleSet2D" => new VRML::NodeType("TriangleSet2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vertices => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__texCoords  =>["MFVec2f",[],"initializeOnly", 0,"UNCA_NONE"],
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
 	],"X3DGeometryNode"),

	###################################################################################

	#	Chapter 15:		Text Component

	###################################################################################

	"Text" => new VRML::NodeType ("Text", [
		fontStyle => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		length => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxExtent => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		string => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		lineBounds => ["MFVec2f",[],"outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		origin => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textBounds => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_isScreen => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"], # > 0 for screenfont
		_screendata => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"], # screentext rowvec
	],"X3DTextNode"),

	"FontStyle" => new VRML::NodeType("FontStyle", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		family => ["MFString", ["SERIF"], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		horizontal => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		justify => ["MFString", ["BEGIN"], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		language => ["SFString", "", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		leftToRight => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		spacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		style => ["SFString", "PLAIN", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		topToBottom => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DFontStyleNode"),

	###################################################################################

	#	Chapter 16:		Sound Component

	###################################################################################

	"AudioClip" => new VRML::NodeType("AudioClip", [
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		loop =>	["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pauseTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pitch => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		resumeTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		startTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration_changed => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		elapsedTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isPaused => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		# internal sequence number, openal buffer number
		__sourceNumber => ["SFInt32", -1, "initializeOnly", 0,"UNCA_NONE"],
		# time that we were initialized at
		__inittime => ["SFTime", 0, "initializeOnly", 0,"UNCA_NONE"],
		__lasttime => ["SFTime", 0, "initializeOnly", 0,"UNCA_NONE"],
		# local name, as received on system
		# old audio __localFileName => ["FreeWRLPTR", 0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DSoundSourceNode"),

	"Sound" => new VRML::NodeType("Sound", [
		direction => ["SFVec3f", [0, 0, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		location => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxBack => ["SFFloat", 10.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxFront => ["SFFloat", 10.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minBack => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minFront => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		priority => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		source => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		spatialize => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# openal sound source number
		__sourceNumber => ["SFInt32", -1, "initializeOnly", 0,"UNCA_NONE"],
		__lastlocation => ["SFVec3f", [0, 0, 0], "initializeOnly",0,"UNCA_NONE"],
		__lasttime => ["SFTime", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DSoundSourceNode"),


	###################################################################################

	# Chapter 17:		Lighting Component

	###################################################################################

	"DirectionalLight" => new VRML::NodeType("DirectionalLight", [
		ambientIntensity => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		direction => ["SFVec3f", [0, 0, -1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		global => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		on => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_dir =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_col =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_amb =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
	],"X3DLightNode"),

	"PointLight" => new VRML::NodeType("PointLight", [
		ambientIntensity => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attenuation => ["SFVec3f", [1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		global => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		location => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		on => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radius => ["SFFloat", 100.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_loc =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_col =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_amb =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
	],"X3DLightNode"),

	"SpotLight" => new VRML::NodeType("SpotLight", [
		ambientIntensity => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attenuation => ["SFVec3f", [1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		beamWidth => ["SFFloat", 1.570796, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		cutOffAngle => ["SFFloat", 0.785398, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		direction => ["SFVec3f", [0, 0, -1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		global => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		location => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		on => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radius => ["SFFloat", 100.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_dir =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_loc =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_col =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_amb =>["SFVec4f",[0,0,0,0],"initializeOnly",0,"UNCA_NONE"],
	],"X3DLightNode"),

	###################################################################################

	#	Chapter18:	Texturing Component

	###################################################################################

	"ImageTexture" => new VRML::NodeType("ImageTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatS => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatT => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DTextureNode"),

	"MovieTexture" => new VRML::NodeType ("MovieTexture", [
		#SoundSource / AudioClip compatible section, keep in same order as AudioClip
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		loop => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pauseTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pitch => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		resumeTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		startTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [""], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration_changed => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		elapsedTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isPaused => ["SFBool","FALSE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		# internal sequence number
		__sourceNumber => ["SFInt32", -1, "initializeOnly", 0,"UNCA_NONE"],
		# time that we were initialized at
		__inittime => ["SFTime", 0, "initializeOnly", 0,"UNCA_NONE"],
		__lasttime => ["SFTime", 0, "initializeOnly", 0,"UNCA_NONE"],
		#Texture2D and Movie section
		repeatS => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatT => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		speed => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		 __frac => ["SFFloat", 0.0, "initializeOnly", 0,"UNCA_NONE"],		
		 # which texture number is used
		 __ctex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		 # lowest frame
		 __lowest => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		 # highest frame
		 __highest => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		 __fw_movie  => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DTextureNode"),


	"MultiTexture" => new VRML::NodeType("MultiTexture", [
		alpha =>["SFFloat", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color =>["SFColor",[1,1,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		function =>["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mode =>["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		source =>["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texture=>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__xparams => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DTextureNode"),

	"MultiTextureCoordinate" => new VRML::NodeType("MultiTextureCoordinate", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord =>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureCoordinateNode"),

	"MultiTextureTransform" => new VRML::NodeType("MultiTextureTransform", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureTransform=>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureTransformNode"),

	"PixelTexture" => new VRML::NodeType("PixelTexture", [
		image => ["SFImage", "0, 0, 0", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatS => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatT => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DTextureNode"),

	"TextureCoordinate" => new VRML::NodeType("TextureCoordinate", [
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		point => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureCoordinateNode"),

	"TextureCoordinateGenerator" => new VRML::NodeType("TextureCoordinateGenerator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mode => ["SFString","SPHERE","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		parameter => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureCoordinateNode"),

	"TextureProperties" => new VRML::NodeType("TextureProperties", [
		anisotropicDegree => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		borderColor=>["SFColorRGBA",[0,0,0,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		borderWidth => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		boundaryModeS => ["SFString", "REPEAT", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		boundaryModeT => ["SFString", "REPEAT", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		boundaryModeR => ["SFString", "REPEAT", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		magnificationFilter => ["SFString", "FASTEST", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minificationFilter => ["SFString", "FASTEST", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureCompression => ["SFString", "FASTEST", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texturePriority => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		generateMipMaps => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

	], "X3DSFNode"),

	"TextureTransform" => new VRML::NodeType ("TextureTransform", [
		center => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec2f", [1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureTransformNode"),


	###################################################################################

	#	Chapter 19:		Interpolation Component

	###################################################################################

	"ColorInterpolator" => new VRML::NodeType("ColorInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFColor", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFColor", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"CoordinateInterpolator" => new VRML::NodeType("CoordinateInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_GPU_Routes_out => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_CPU_Routes_out => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],

		# GPU running only - run the interpolator on the GPU, use these...
		_keyVBO =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_keyValueVBO =>["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"CoordinateInterpolator2D" => new VRML::NodeType("CoordinateInterpolator2D", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["MFVec2f", [[0,0]], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"EaseInEaseOut" => new VRML::NodeType("EaseInEaseOut", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		easeInEaseOut => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		modifiedFraction_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),



	"NormalInterpolator" => new VRML::NodeType("NormalInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"OrientationInterpolator" => new VRML::NodeType("OrientationInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFRotation", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"PositionInterpolator" => new VRML::NodeType("PositionInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"PositionInterpolator2D" => new VRML::NodeType("PositionInterpolator2D", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"ScalarInterpolator" => new VRML::NodeType("ScalarInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"SplinePositionInterpolator" => new VRML::NodeType("SplinePositionInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		closed => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyVelocity => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFVec3f", [0,0,0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_T0 => ["MFVec3f", [], "initializeOnly", 0,"UNCA_NONE"],
		_T1 => ["MFVec3f", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"SplinePositionInterpolator2D" => new VRML::NodeType("SplinePositionInterpolator2D", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		closed => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyVelocity => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFVec2f", [0,0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_T0 => ["MFVec2f", [], "initializeOnly", 0,"UNCA_NONE"],
		_T1 => ["MFVec2f", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"SplineScalarInterpolator" => new VRML::NodeType("SplineScalarInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		closed => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyVelocity => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_T0 => ["MFFloat", [], "initializeOnly", 0,"UNCA_NONE"],
		_T1 => ["MFFloat", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DInterpolatorNode"),

	"SquadOrientationInterpolator" => new VRML::NodeType("SquadOrientationInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFRotation", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		closed => ["SFBool", "FALSE", "inputOutput", 0,"UNCA_NONE"], #H: the specs made a mistake it should be 'closed' not 'normalizeVelocity' -dug9
		value_changed => ["SFRotation", [0,0,1,0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_normkey => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_normkeyValue => ["MFRotation", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
  	],"X3DInterpolatorNode"),

	###################################################################################

	#		Cubemap Texturing Component

	###################################################################################


	"ComposedCubeMapTexture" => new VRML::NodeType("ComposedCubeMapTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		back =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bottom =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		front =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		left =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		top =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		right =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DEnvironmentTextureNode"),

	"GeneratedCubeMapTexture" => new VRML::NodeType("GeneratedCubeMapTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__subTextures => ["MFNode",[],"initializeOnly",0,"UNCA_NONE"],
		__regenSubTextures => ["SFBool","FALSE","initializeOnly",0,"UNCA_NONE"],
		update => ["SFString","NONE","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFInt32",128,"initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
	],"X3DEnvironmentTextureNode"),

	#same order of fields up to __regenSubtextures as GeneratedCubeMapTexture
	"ImageCubeMapTexture" => new VRML::NodeType("ImageCubeMapTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__subTextures => ["MFNode",[],"initializeOnly",0,"UNCA_NONE"],
		__regenSubTextures => ["SFBool","FALSE","initializeOnly",0,"UNCA_NONE"],
		url => ["MFString",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DEnvironmentTextureNode"),




	###################################################################################

	#	20	Pointing Device Component

	###################################################################################

	"TouchSensor" => new VRML::NodeType("TouchSensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hitNormal_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hitPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hitTexCoord_changed => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldhitNormal => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"], 	# send event only if changed
		_oldhitPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"], 	# send event only if changed
		_oldhitTexCoord => ["SFVec2f", [0, 0], "outputOnly", 0,"UNCA_NONE"], 	# send event only if changed
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		touchTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DPointingDeviceSensorNode"),

	"PlaneSensor" => new VRML::NodeType("PlaneSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axisRotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxPosition => ["SFVec2f", [-1, -1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minPosition => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		offset => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"],
		_oldtranslation => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"],
		# where we are at a press...
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DPointingDeviceSensorNode"),

#
# Experimental node: LineSensor
#
	"LineSensor" => new VRML::NodeType("LineSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		direction => ["SFVec3f", [1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxPosition => ["SFFloat", -1, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minPosition => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		offset => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"],
		_oldtranslation => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"],
		# where we are at a press...
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DPointingDeviceSensorNode"),

	"SphereSensor" => new VRML::NodeType("SphereSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		offset => ["SFRotation", [0, 1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"],
		_oldrotation => ["SFRotation", [0, 0, 1, 0], "outputOnly", 0,"UNCA_NONE"],
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		# where we are at a press...
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,"UNCA_NONE"],
		_origNormalizedPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,"UNCA_NONE"],
		_radius => ["SFFloat", 0, "initializeOnly", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DPointingDeviceSensorNode"),

	"CylinderSensor" => new VRML::NodeType("CylinderSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axisRotation => ["SFRotation", [0, 1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		diskAngle => ["SFFloat", 0.262, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxAngle => ["SFFloat", -1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minAngle => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		offset => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		rotation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"],
		_oldrotation => ["SFRotation", [0, 0, 1, 0], "outputOnly", 0,"UNCA_NONE"],
		# where we are at a press...
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,"UNCA_NONE"],
		_radius => ["SFFloat", 0, "initializeOnly", 0,"UNCA_NONE"],
		_dlchange => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DPointingDeviceSensorNode"),


	###################################################################################

	#	21	Key Device Component

	###################################################################################

	# KeySensor
	"KeySensor" => new VRML::NodeType("KeySensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		actionKeyPress =>["SFInt32",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		actionKeyRelease =>["SFInt32",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		altKey =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlKey =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyPress =>["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyRelease =>["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		shiftKey =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DKeyDeviceSensorNode"),

	# StringSensor
	"StringSensor" => new VRML::NodeType("StringSensor", [
		deletionAllowed => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enteredText => ["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		finalText => ["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_initialized =>["SFBool", "FALSE","initializeOnly", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DKeyDeviceSensorNode"),


	###################################################################################

	#	22	Environmental Sensor Component

	###################################################################################


	"ProximitySensor" => new VRML::NodeType("ProximitySensor", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orientation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		centerOfRotation_changed =>["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# These fields are used for the info.
		__hit => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"],
		__t1 => ["SFVec3f", [10000000, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__t2 => ["SFRotation", [0, 1, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DEnvironmentalSensorNode"),

	"TransformSensor" => new VRML::NodeType("TransformSensor", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orientation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		targetObject => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# These fields are used for the info.
		__hit => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"],
		__t1 => ["SFVec3f", [10000000, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__t2 => ["SFRotation", [0, 1, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DEnvironmentalSensorNode"),


	"VisibilitySensor" => new VRML::NodeType("VisibilitySensor", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		 __visible =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"], # for Occlusion tests.
		 __occludeCheckCount =>["SFInt32",-1,"initializeOnly", 0,"UNCA_NONE"], # for Occlusion tests.
		__points  =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],	# for Occlude Box.
		__Samples =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],		# Occlude samples from last pass
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DEnvironmentalSensorNode"),



	###################################################################################

	#	23	Navigation Component

	###################################################################################

	"LOD" => new VRML::NodeType("LOD", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		level => ["MFNode", [], "inputOutput", "(SPEC_VRML)","UNCA_NONE"], 		# for VRML spec
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],		# for X3D spec
		center => ["SFVec3f", [0, 0, 0],  "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		range => ["MFFloat", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		levelChanged => ["SFInt32", 0, "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceTransitions => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__isX3D => ["SFBool", "(inputFileVersion[0]==3)" , "initializeOnly", 0,"UNCA_NONE"], # "TRUE" for X3D V3.x files
		_selected =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),

	"Billboard" => new VRML::NodeType("Billboard", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axisOfRotation => ["SFVec3f", [0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_rotationAngle =>["SFDouble", 0, "initializeOnly", 0,"UNCA_NONE"],
		#JAS _sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),

	"Collision" => new VRML::NodeType("Collision", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		collide => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		proxy => ["SFNode", "NULL", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		collideTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		#JAS _sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		# return info for collisions
		# bit 0 : collision or not
		# bit 1: changed from previous of not
		__hit => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"],
	],"X3DEnvironmentalSensorNode"),


	"Viewpoint" => new VRML::NodeType("Viewpoint", [
		#generic Viewpoint fields
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		jump => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fieldOfView => ["SFFloat", 0.785398, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position => ["SFVec3f",[0, 0, 10], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_layerId => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_donethispass => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		centerOfRotation =>["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		retainUserOffsets => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# augmented reality extensions:
		fovMode => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		aspectRatio => ["SFFloat", 0.785398, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		
	],"X3DBindableNode"),

	"OrthoViewpoint" => new VRML::NodeType("OrthoViewpoint", [
		#generic Viewpoint fields
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		jump => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fieldOfView => ["MFFloat", [-1.0, -1.0, 1.0, 1.0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position => ["SFVec3f",[0, 0, 10], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_layerId => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_donethispass => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		centerOfRotation =>["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		retainUserOffsets => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DBindableNode"),



	"NavigationInfo" => new VRML::NodeType("NavigationInfo", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		avatarSize => ["MFFloat", [0.25, 1.6, 0.75], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		headlight => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		type => ["MFString", ["EXAMINE", "ANY"], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		visibilityLimit => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_layerId => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		transitionType => ["MFString", [],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transitionTime => ["SFTime", 1.0, "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transitionComplete => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DBindableNode"),

	"ViewpointGroup" => new VRML::NodeType("ViewpointGroup", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		displayed => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		retainUserOffsets => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__proxNode=> ["SFNode", "NULL", "inputOutput", "0","UNCA_NONE"],
	],"X3DGroupingNode"),



	###################################################################################

	#	24	Environmental Effects Component

	###################################################################################

	"Background" => new VRML::NodeType("Background", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		groundAngle => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		groundColor => ["MFColor", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skyAngle => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skyColor => ["MFColor", [[0, 0, 0]], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_layerId => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__points =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		__colours =>["MFColor",[],"initializeOnly", 0,"UNCA_NONE"],
		__quadcount => ["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],

		transparency => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		frontUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		backUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		topUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bottomUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		leftUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rightUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureright => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"],
		__frontTexture=>["SFNode","NULL","inputOutput", 0,"UNCA_NONE"],
		__backTexture=>["SFNode","NULL","inputOutput", 0,"UNCA_NONE"],
		__topTexture=>["SFNode","NULL","inputOutput", 0,"UNCA_NONE"],
		__bottomTexture=>["SFNode","NULL","inputOutput", 0,"UNCA_NONE"],
		__leftTexture=>["SFNode","NULL","inputOutput", 0,"UNCA_NONE"],
		__rightTexture=>["SFNode","NULL","inputOutput", 0,"UNCA_NONE"],

		__VBO=>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],  # Vertex Buffer Object, if required.
	],"X3DBackgroundNode"),



	"Fog" => new VRML::NodeType("Fog", [
		#Fog interface - keep same order, offsets as LocalFog
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogType => ["SFString", "LINEAR", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		visibilityRange => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__fogScale => ["SFFloat", 1.0, "inputOutput", 0,"UNCA_NONE"],
		__fogType => ["SFInt32",1,"initializeOnly",0,"UNCA_NONE"],
		#Bindable interface
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_layerId => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	  ],"X3DBindableNode"),

	"FogCoordinate" => new VRML::NodeType("FogCoordinate", [
		depth => ["MFFloat", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DGeometricPropertyNode"),

	"LocalFog" => new VRML::NodeType("Fog", [
		#Fog interface - keep same order, offsets as Fog
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogType => ["SFString", "LINEAR", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		visibilityRange => ["SFFloat", 0, "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__fogScale => ["SFFloat", 1.0, "inputOutput", 0,"UNCA_NONE"],
		__fogType => ["SFInt32",1,"initializeOnly",0,"UNCA_NONE"],
		#other
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"TextureBackground" => new VRML::NodeType("TextureBackground", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		groundAngle => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		groundColor => ["MFColor", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skyAngle => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skyColor => ["MFColor", [[0,0,0]], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_layerId => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__points =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		__colours =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		__quadcount => ["SFInt32",0,"initializeOnly", 0],
		__VBO=>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],  # Vertex Buffer Object, if required.

		frontTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		backTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		topTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bottomTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		leftTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rightTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transparency=> ["MFFloat",[0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DBackgroundNode"),

	# see augmented reality for 2 more background nodes
	
	
	###################################################################################

	#	25	Geospatial Component

	###################################################################################


	"GeoCoordinate" => new VRML::NodeType("GeoCoordinate", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		point => ["MFVec3d",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedCoords => ["MFVec3f", [], "inputOutput", 0,"UNCA_NONE"],
	],"X3DCoordinateNode"),

	"GeoElevationGrid" => new VRML::NodeType("GeoElevationGrid", [
		set_height => ["MFDouble", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		yScale => ["SFFloat", 1.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		creaseAngle => ["SFDouble", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoGridOrigin => ["SFVec3d",[0,0,0],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		height => ["MFDouble", [0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		xDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		xSpacing => ["SFDouble", 1.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		zDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		zSpacing => ["SFDouble", 1.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],

		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
	],"X3DGeometryNode"),

	"GeoLOD" => new VRML::NodeType("GeoLOD", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# the following screws up routing in the old VRML parser, because children can
		# be an "EXPOSED_FIELD" AND an "EVENT_OUT", so by changing this to an ""inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"
		# we can have only one field, the EXPOSED_FIELD_children
		#children => ["MFNode",[],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		children => ["MFNode", [], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		level_changed =>["SFInt32",0,"outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		center => ["SFVec3d",[0,0,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		child1Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		child2Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		child3Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		child4Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		range => ["SFFloat",10.0,"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rootUrl => ["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rootNode => ["MFNode",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__inRange =>["SFBool", "FALSE", "inputOutput", 0,"UNCA_NONE"],
		__child1Node => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__child2Node => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__child3Node => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__child4Node => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__rootUrl => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__childloadstatus => ["SFInt32",0,"inputOutput", 0,"UNCA_NONE"],
		__rooturlloadstatus => ["SFInt32",0,"inputOutput", 0,"UNCA_NONE"],

		# ProximitySensor copies.
		#__t1 => ["SFVec3d", [10000000, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__level => ["SFInt32",-1,"inputOutput", 0,"UNCA_NONE"], # only for debugging purposes
	],"X3DGroupingNode"),


	"GeoMetadata" => new VRML::NodeType("GeoMetadata", [
		data => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		summary => ["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"GeoPositionInterpolator" => new VRML::NodeType("GeoPositionInterpolator", [
		set_fraction => ["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFVec3d",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geovalue_changed => ["SFVec3d",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedValue => ["MFVec3d", [], "inputOutput", 0,"UNCA_NONE"],
		__oldKeyPtr => ["MFFloat", "NULL", "outputOnly", 0,"UNCA_NONE"],
		__oldKeyValuePtr => ["MFVec3d", "NULL", "outputOnly", 0,"UNCA_NONE"],
	],"X3DInterpolatorNode"),


	"GeoProximitySensor" => new VRML::NodeType("ProximitySensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		geoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D32)","UNCA_NONE"],
		center => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		centerOfRotation_changed =>["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		geoCoord_changed => ["SFVec3d",[0,0,0],"outputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		orientation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		position_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32)","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],


		# These fields are used for the info.
		__hit => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"],
		__t1 => ["SFVec3f", [10000000, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__t2 => ["SFRotation", [0, 1, 0, 0], "inputOutput", 0,"UNCA_NONE"],

		# "compiled" versions of strings above
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__localOrient => ["SFVec4d", [0, 0, 1, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
		__oldGeoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldSize => ["SFVec3f", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
	],"X3DEnvironmentalSensorNode"),

	"GeoTouchSensor" => new VRML::NodeType("GeoTouchSensor", [
		description => ["SFString", "", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hitNormal_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hitPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hitTexCoord_changed => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hitGeoCoord_changed => ["SFVec3d", [0, 0, 0] ,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		touchTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		_oldhitNormal => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"], 	# send event only if changed
		_oldhitPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,"UNCA_NONE"], 	# send event only if changed
		_oldhitTexCoord => ["SFVec2f", [0, 0], "outputOnly", 0,"UNCA_NONE"], 	# send event only if changed
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DPointingDeviceSensorNode"),


	"GeoTransform" => new VRML::NodeType ("GeoTransform", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		geoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32)","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],

		# fields for reducing redundant calls
		__do_center => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_trans => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_rotation => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_scaleO => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_scale => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],

		# "compiled" versions of strings above
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__localOrient => ["SFVec4d", [0, 0, 1, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldGeoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),

	"GeoViewpoint" => new VRML::NodeType("GeoViewpoint", [
		# generic Viewpoint fields
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		jump => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fieldOfView => ["SFFloat", 0.785398, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file, AND spec changed to in/out in 3.3
		position => ["SFVec3d",[0, 0, 100000], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # ditto
		centerOfRotation => ["SFVec3d",[0, 0, 0], "inputOutput", "( SPEC_X3D33)","UNCA_NONE"],
		_layerId => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_donethispass => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		set_orientation => ["SFRotation", ["IO_FLOAT", "IO_FLOAT", "IO_FLOAT", "IO_FLOAT"], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		set_position => ["SFVec3d", ["IO_FLOAT", "IO_FLOAT", "IO_FLOAT"], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		# GeoViewpoint fields
		headlight => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		navType => ["MFString", ["EXAMINE","ANY"],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speedFactor => ["SFFloat",1.0,"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		retainUserOffsets => ["SFBool", "FALSE", "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		

		# "compiled" versions of strings above
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedPosition => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__movedOrientation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,"UNCA_NONE"],

		__oldSFString => ["SFString", "", "inputOutput", 0,"UNCA_NONE"], #the description field
		__oldFieldOfView => ["SFFloat", 0.785398, "inputOutput", 0,"UNCA_NONE"],
		__oldHeadlight => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
		__oldJump => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
		__oldMFString => ["MFString", [],"inputOutput", 0,"UNCA_NONE"], # the navType

	],"X3DBindableNode"),

	"GeoOrigin" => new VRML::NodeType("GeoOrigin", [
		geoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotateYUp => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# these are now static in CFuncs/GeoVRML.c
		# "compiled" versions of strings above
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldgeoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldMFString => ["MFString", [],"inputOutput", 0,"UNCA_NONE"], # the navType
		__rotyup => ["SFVec4d", [0, 1, 0, 0], "inputOutput", 0,"UNCA_NONE"],

	],"X3DChildNode"),

	"GeoLocation" => new VRML::NodeType("GeoLocation", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# "compiled" versions of strings above
		__geoSystem => ["MFInt32",[],"initializeOnly", 0,"UNCA_NONE"],
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__localOrient => ["SFVec4d", [0, 0, 1, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldgeoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		__oldChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),


	###################################################################################

	#	26	H-Anim Component

	###################################################################################

	"HAnimDisplacer" => new VRML::NodeType("HAnimDisplacer", [
		coordIndex => ["MFInt32", [], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		displacements => ["MFVec3f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"],
		weight => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DGeometricPropertyNode"),

	"HAnimHumanoid" => new VRML::NodeType("HAnimHumanoid", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		info => ["MFString", [],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		joints => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation",[0,0,1,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f",[1,1,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		segments => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sites => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skeleton => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skin => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skinCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skinNormal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		version => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		viewpoints => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_JT => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_PVI => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_PVW => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],	
		_NV => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_origCoords => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_origNorms => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DChildNode"),

	"HAnimJoint" => new VRML::NodeType("HAnimJoint", [

		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		displacers => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		limitOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		llimit => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skinCoordIndex => ["MFInt32",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		skinCoordWeight => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stiffness => ["MFFloat",[0,0,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ulimit => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# fields for reducing redundant calls
		__do_center => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_trans => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_rotation => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_scaleO => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_scale => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_anything => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChildNode"),

	"HAnimSegment" => new VRML::NodeType("HAnimSegment", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		centerOfMass => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		displacers => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass => ["SFFloat", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		momentsOfInertia =>["MFFloat", [0, 0, 0, 0, 0, 0, 0, 0, 0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_origCoords => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DChildNode"),



	"HAnimSite" => new VRML::NodeType("HAnimSite", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# fields for reducing redundant calls
		__do_center => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_trans => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_rotation => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_scaleO => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_scale => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__do_anything => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DGroupingNode"),


	###################################################################################

	#	27	NURBS Component

	###################################################################################

	"Contour2D" => new VRML::NodeType("Contour2D", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DSFNode"),


	"ContourPolyline2D" => new VRML::NodeType("ContourPolyline2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint => ["MFVec2d", [], "inputOutput","( SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],  #v3.1 changed to this
		point => ["MFVec2f", [], "inputOutput","(SPEC_X3D30 )","UNCA_NONE"], #from this
	],"X3DNurbsControlCurveNode"),

	"CoordinateDouble" => new VRML::NodeType("CoordinateDouble", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		point => ["MFVec3d", [], "inputOutput","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DCoordinateNode"),

	"NurbsCurve" => new VRML::NodeType("NurbsCurve", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		knot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tessellation => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		closed => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,"UNCA_NONE"],
		__points  =>["MFVec3f",[],"initializeOnly", 0,"UNCA_NONE"],
		__numPoints =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DParametricGeometryNode"),

	"NurbsCurve2D" => new VRML::NodeType("NurbsCurve2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["MFVec2d",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		knot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tessellation => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		closed => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DNurbsControlCurveNode"),


	"NurbsOrientationInterpolator" => new VRML::NodeType("NurbsOrientationInterpolator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight  => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		knot => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_fraction => ["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFRotation",[0,0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_knot => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_xyzw => ["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_OK => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_knotrange => ["SFVec2f", [0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"NurbsPatchSurface" => new VRML::NodeType("NurbsPatchSurface", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DNurbsSurfaceGeometryNode"),

	"NurbsPositionInterpolator" => new VRML::NodeType("NurbsPositionInterpolator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		knot => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_fraction => ["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_knot => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_xyzw => ["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_OK => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_knotrange => ["SFVec2f", [0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"NurbsSet" => new VRML::NodeType("NurbsSet", [
		addGeometry => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeGeometry => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tessellationScale => ["SFFloat",1.0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"NurbsSurfaceInterpolator" => new VRML::NodeType("NurbsSurfaceInterpolator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_fraction => ["SFVec2f",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_uKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_vKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_controlPoint =>["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_OK => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"NurbsSweptSurface" => new VRML::NodeType("NurbsSweptSurface", [
		crossSectionCurve =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		trajectoryCurve => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		method => ["SFString", "FULL", "inputOnly", 0,"UNCA_NONE"], #TRANSLATE / FULL
		_patch => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_method => ["SFInt32",2,"initializeOnly",0,"UNCA_NONE"], #1. Suv = Tv + Cu and delegate to patch 2. insert xsection at each profile tess point, and skin
	],"X3DParametricGeometryNode"),

	"NurbsSwungSurface" => new VRML::NodeType("NurbsSwungSurface", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		profileCurve =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		trajectoryCurve => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_patch => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
	],"X3DParametricGeometryNode"),

	"NurbsTextureCoordinate" => new VRML::NodeType("NurbsTextureCoordinate", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["MFVec2f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_uKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_vKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_controlPoint =>["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DSFNode"),

	#TrimmedSurface == PatchSurface + trimmingContour - keep them in the same order so Trimmed can be downcast to Patch
	"NurbsTrimmedSurface" => new VRML::NodeType("NurbsTrimmedSurface", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		uClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		vClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		addTrimmingContour => ["MFNode",[],"inputOnly","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeTrimmingContour => ["MFNode",[],"inputOnly","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		trimmingContour =>["MFNode",[], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,"UNCA_NONE"],
	], "X3DNurbsSurfaceGeometryNode"),


	###################################################################################

	# Chapter 28: Distributed Interactive Simulation Component

	###################################################################################


	"DISEntityManager" => new VRML::NodeType("DISEntityManager", [
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mapping => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		addedEntities => ["MFNode", [], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removedEntities => ["MFNode", [], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),

	"DISEntityTypeMapping" => new VRML::NodeType("DISEntityTypeMapping", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		category => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		country => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		domain => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		extra => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		kind => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		specific => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		subcategory => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

	],"X3DInfoNode"),


	"EspduTransform" => new VRML::NodeType("EspduTransform", [
		addChildren => ["MFNode", [], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", [], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue0 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue1 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue2 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue3 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue4 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue5 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue6 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_articulationParameterValue7 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterCount => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterDesignatorArray => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterChangeIndicatorArr => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterIdPartAttachedToAr => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterTypeArray => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterArray => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		center => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		collisionType => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		deadReckoning => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		detonationLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		detonationRelativeLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		detonationResult => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityCategory => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityCountry => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityDomain => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityExtra => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityKind => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entitySpecific => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entitySubCategory => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		eventApplicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		eventEntityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		eventNumber => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		eventSiteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fired1 => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fired2 => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fireMissionIndex => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		firingRange => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		firingRate => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fuse => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],
		linearVelocity => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		linearAcceleration => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		marking => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		munitionApplicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		munitionEndPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		munitionEntityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		munitionQuantity => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		munitionSiteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		munitionStartPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		readInterval => ["SFTime", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0,0,1,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f", [1,1,1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scaleOrientation => ["SFRotation", [0,0,1,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		warhead => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		writeInterval => ["SFTime", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue0_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue1_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue2_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue3_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue4_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue5_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue6_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		articulationParameterValue7_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		collideTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		detonateTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		firedTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isCollided => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isDetonated => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		timestamp => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rtpHeaderExpected => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DGroupingNode"),


	"ReceiverPdu" => new VRML::NodeType("ReceiverPdu", [
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		readInterval => ["SFFloat", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		receivedPower => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		receiverState => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rtpHeaderExpected => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transmitterApplicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transmitterEntityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transmitterRadioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transmitterSiteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		whichGeometry => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		writeInterval => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		timestamp => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DChildNode"),

	"SignalPdu" => new VRML::NodeType("SignalPdu", [
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		data => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		dataLength => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		encodingScheme => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		readInterval => ["SFFloat", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rtpHeaderExpected => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sampleRate => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		samples => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tdlType => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		whichGeometry => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		writeInterval => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		timestamp => ["SFTime", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DChildNode"),

	"TransmitterPdu" => new VRML::NodeType("TransmitterPdu", [
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		antennaLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		antennaPatternLength => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		antennaPatternType => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		cryptoKeyID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		cryptoSystem => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		frequency => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],
		inputSource => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		lengthOfModulationParameters => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		modulationTypeDetail => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		modulationTypeMajor => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		modulationTypeSpreadSpectrum => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		modulationTypeSystem => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		power => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioEntityTypeCategory => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioEntityTypeCountry => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioEntityTypeDomain => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioEntityTypeKind => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioEntityTypeNomenclature => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioEntityTypeNomenclatureVersion => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		radioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		readInterval => ["SFFloat", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		relativeAntennaLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rtpHeaderExpected => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transmitFrequencyBandwidth => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transmitState => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		whichGeometry => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		writeInterval => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		timestamp => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DChildNode"),





	###################################################################################

	#	29.	Scripting Component

	###################################################################################
	"Script" => new VRML::NodeType("Script", [
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		directOutput => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mustEvaluate => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__scriptObj => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DScriptNode"),

	###################################################################################

	#	32.	CAD Component

	###################################################################################

	"CADAssembly" => new VRML::NodeType("CADAssembly", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	], "X3DGroupingNode"),

	"CADFace" => new VRML::NodeType("CADFace", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		shape => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DProductStructureChildNode"),

	"CADLayer" => new VRML::NodeType("CADLayer", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		visible => ["MFBool", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DGroupingNode"),

	"CADPart" => new VRML::NodeType("CADPart", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		center => ["SFVec3f",[0,0,0],"inputOutput","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		# fields for reducing redundant calls
		__do_center => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_scaleO => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_scale => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_anything => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		_sortedChildren => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
	] ,"X3DGroupingNode"),

	"IndexedQuadSet" => new VRML::NodeType("IndexedQuadSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		attrib  => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		index => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	], "X3DComposedGeometryNode"),

	"QuadSet" => new VRML::NodeType("QuadSet", [
		attrib  => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,"UNCA_NONE"],
	], "X3DComposedGeometryNode"),


	###################################################################################

	#	30.	EventUtilities Component

	###################################################################################

	"BooleanFilter" => new VRML::NodeType("BooleanFilter", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		inputFalse => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		inputNegate => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		inputTrue => ["SFBool", "TRUE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),


	"BooleanSequencer" => new VRML::NodeType("BooleanSequencer", [
		next =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		previous =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_fraction =>["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFBool", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DSequencerNode"),


	"BooleanToggle" => new VRML::NodeType("BooleanToggle", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		toggle => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DChildNode"),


	"BooleanTrigger" => new VRML::NodeType("BooleanTrigger", [
		set_triggerTime => ["SFTime",undef ,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		triggerTrue => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTriggerNode"),


	"IntegerSequencer" => new VRML::NodeType("IntegerSequencer", [
		next =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		previous =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_fraction =>["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		keyValue => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value_changed => ["SFInt32", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DSequencerNode"),

	"IntegerTrigger" => new VRML::NodeType("IntegerTrigger", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		integerKey => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		triggerValue => ["SFInt32", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTriggerNode"),

	"TimeTrigger" => new VRML::NodeType("TimeTrigger", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		triggerTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTriggerNode"),


	###################################################################################

	#	31.	ProgrammableShaders Component

	###################################################################################

	"ComposedShader" => new VRML::NodeType("ComposedShader", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		parts => ["MFNode",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		language => ["SFString", "", "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,"UNCA_NONE"],
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,"UNCA_NONE"],
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DShaderNode"),


	"FloatVertexAttribute" => new VRML::NodeType("FloatVertexAttribute", [
		value => ["MFFloat",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		numComponents => ["SFInt32", 4, "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # 1...4 valid values
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DVertexAttributeNode"),

	"Matrix3VertexAttribute" => new VRML::NodeType("Matrix3VertexAttribute", [
		value => ["MFMatrix3f",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DVertexAttributeNode"),

	"Matrix4VertexAttribute" => new VRML::NodeType("Matrix4VertexAttribute", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		value => ["MFMatrix4f",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		name => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DVertexAttributeNode"),

	"PackagedShader" => new VRML::NodeType("PackagedShader", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		language => ["SFString","","initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,"UNCA_NONE"],
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,"UNCA_NONE"],
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
	], "X3DProgrammableShaderObject"),

	"ProgramShader" => new VRML::NodeType("ProgramShader", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		programs => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		language => ["SFString","","initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,"UNCA_NONE"],
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,"UNCA_NONE"],
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
	], "X3DProgrammableShaderObject"),

	"ShaderPart" => new VRML::NodeType("ShaderPart", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		type => ["SFString","VERTEX","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
	], "X3DUrlObject"),

	"ShaderProgram" => new VRML::NodeType("ShaderProgram", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		type => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
	], "X3DUrlObject"),

	# castle EffectPart made from ShaderPart - fields in same order
	"EffectPart" => new VRML::NodeType("EffectPart", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		type => ["SFString","VERTEX","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
	], "X3DUrlObject"),

	# castle Effect made from ComposedShader - fields in same order
	"Effect" => new VRML::NodeType("Effect", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		parts => ["MFNode",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		language => ["SFString", "", "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,"UNCA_NONE"],
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,"UNCA_NONE"],
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DShaderNode"),


	###################################################################################

	#	33.	Texturing3D Component

	###################################################################################
	"ImageTexture3D" => new VRML::NodeType("ImageTexture3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatS => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatT => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatR => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_needs_gradient => ["SFBool", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
	],"X3DTextureNode"),

	"PixelTexture3D" => new VRML::NodeType("PixelTexture3D", [
		image => ["MFInt32", "0, 0, 0, 0", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatS => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatT => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatR => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_needs_gradient => ["SFBool", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
	],"X3DTextureNode"),

	"TextureCoordinate3D" => new VRML::NodeType("TextureCoordinate3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		point => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureCoordinateNode"),

	"TextureCoordinate4D" => new VRML::NodeType("TextureCoordinate4D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		point => ["MFVec4f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureCoordinateNode"),

	"TextureTransformMatrix3D" => new VRML::NodeType("TextureTransformMatrix3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		matrix => ["SFMatrix4f", [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureTransformNode"),

	"TextureTransform3D" => new VRML::NodeType ("TextureTransform3D", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DTextureTransformNode"),

	"ComposedTexture3D" => new VRML::NodeType("ComposedTexture3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texture=>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],
		repeatS => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatT => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		repeatR => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
	],"X3DTexture3DNode"),


	###################################################################################

	#	35.	Layering Component

	###################################################################################

	"Viewport" => new VRML::NodeType("Viewport", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		clipBoundary => ["MFFloat",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DViewportNode"),

	"Layer" => new VRML::NodeType("Layer", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isPickable => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		viewport => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DLayerNode"),

	"LayerSet" => new VRML::NodeType("LayerSet", [
		activeLayer => ["SFInt32", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		layers => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["MFInt32",[0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DLayerSetNode"),

	###################################################################################

	#	36.	Layout Component

	###################################################################################
	"Layout" => new VRML::NodeType("Layout", [
		align => ["MFString", ["CENTER","CENTER"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		offset => ["MFFloat",[0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		offsetUnits => ["MFString", ["WORLD","WORLD"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		scaleMode => ["MFString", ["NONE","NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		size => ["MFFloat",[1,1],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sizeUnits => ["MFString", ["WORLD","WORLD"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_align => ["MFInt32",[0,0],"initializeOnly", 0,"UNCA_NONE"],
		_offsetUnits => ["MFInt32",[0,0], "initializeOnly", 0,"UNCA_NONE"],
		_scaleMode => ["MFInt32",[0,0], "initializeOnly", 0,"UNCA_NONE"],
		_sizeUnits => ["MFInt32",[0,0], "initializeOnly", 0,"UNCA_NONE"],
		_scale => ["MFFloat",[1,1], "initializeOnly", 0,"UNCA_NONE"],
	], "X3DLayoutNode"),

	"LayoutGroup" => new VRML::NodeType("LayoutGroup", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		layout => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		viewport => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DGroupingNode"),



	"LayoutLayer" => new VRML::NodeType("LayoutLayer", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isPickable => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		viewport => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		layout => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DGroupingNode"),

	"ScreenFontStyle" => new VRML::NodeType("ScreenFontStyle", [
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		family => ["MFString", ["SERIF"], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		horizontal => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		justify => ["MFString", ["BEGIN"], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		language => ["SFString", "", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		leftToRight => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pointSize => ["SFFloat", 12.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		spacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		style => ["SFString", "PLAIN", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		topToBottom => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DFontStyleNode"),


	"ScreenGroup" => new VRML::NodeType("ScreenGroup", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DGroupingNode"),

	###################################################################################

	#	37.	Rigid Body Physics Component

	###################################################################################

	"BallJoint" => new VRML::NodeType("BallJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_VRML | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_forceout => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
	], "X3DRigidJointNode"),

	"CollidableOffset" => new VRML::NodeType("CollidableOffset", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		collidable => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_geom => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_initialRotation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,"UNCA_NONE"], 
		_initialTranslation => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,"UNCA_NONE"],
		_initialized => ["SFBool",0,"initializeOnly",0,"UNCA_NONE"],
		_csensor => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	], "X3DNBodyCollidableNode"),

	"CollidableShape" => new VRML::NodeType("CollidableShape", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,"UNCA_NONE"],
		shape => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_geom => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_initialRotation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,"UNCA_NONE"], 
		_initialTranslation => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,"UNCA_NONE"],
		_initialized => ["SFBool",0,"initializeOnly",0,"UNCA_NONE"],
		_csensor => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	], "X3DNBodyCollidableNode"),

	"CollisionCollection" => new VRML::NodeType("CollisionCollection", [
		appliedParameters => ["MFString", ["BOUNCE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bounce => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		collidables => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		frictionCoefficients => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minBounceSpeed => ["SFFloat", 0.1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		slipFactors => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		softnessConstantForceMix => ["SFFloat", 0.0001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		softnessErrorCorrection => ["SFFloat", 0.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
		surfaceSpeed => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_class => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_csensor => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_appliedParametersMask => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChildNode"),

	"CollisionSensor" => new VRML::NodeType("CollsionSensor", [
		collider => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intersections => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		contacts => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "TRUE", "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DSensorNode"),

	"CollisionSpace" => new VRML::NodeType("CollisionSpace", [
		collidables => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		useGeometry => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_space => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	], "X3DNBodyCollidableNode"),

	"Contact" => new VRML::NodeType("Contact", [
		appliedParameters => ["MFString", ["BOUNCE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bounce => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		contactNormal => ["SFVec3f", [0,1,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		depth => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		frictionCoefficients => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		frictionDirection => ["SFVec3f", [0,1,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minBounceSpeed => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		slipCoefficients => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		softnessConstantForceMix => ["SFFloat", 0.0001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		softnessErrorCorrection => ["SFFloat", 0.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceSpeed => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_appliedParameters => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
	], "X3DSFNode"),

	"DoubleAxisHingeJoint" => new VRML::NodeType("DoubleAxisHingeJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis1 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis2 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		desiredAngularVelocity1 => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		desiredAngularVelocity2 => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxAngle1 => ["SFFloat", "PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxTorque1 => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxTorque2 => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minAngle1 => ["SFFloat", "-PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopBounce1 => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopConstantForceMix1 => ["SFFloat", .001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopErrorCorrection1 => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		suspensionErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		suspensionForce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hinge1Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hinge1AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hinge2Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		hinge2AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_forceout => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_axis1 => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_axis2 => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		_motor1 => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_motor2 => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		axis1Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	], "X3DRigidJointNode"),

	"MotorJoint" => new VRML::NodeType("MotorJoint", [
		axis1Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis1Torque => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis2Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis2Torque => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis3Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis3Torque => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabledAxes => ["SFInt32", 1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor1Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor2Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor3Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop1Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop1ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop2Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop2ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop3Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop3ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		motor1Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor1AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor2Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor2AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor3Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		motor3AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		autoCalc => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_forceout => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__old_motor1Axis => ["SFVec3f",[0,0,0],"outputOnly", 0,"UNCA_NONE"],
		__old_motor2Axis => ["SFVec3f",[0,0,0],"outputOnly", 0,"UNCA_NONE"],
		__old_motor3Axis => ["SFVec3f",[0,0,0],"outputOnly", 0,"UNCA_NONE"],
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__old_axis1Angle => ["SFFloat", 0, "inputOutput", 0,"UNCA_NONE"],
		__old_axis2Angle => ["SFFloat", 0, "inputOutput", 0,"UNCA_NONE"],
		__old_axis3Angle => ["SFFloat", 0, "inputOutput", 0,"UNCA_NONE"],
	], "X3DRigidJointNode"),

	"RigidBody" => new VRML::NodeType("RigidBody", [
		angularDampingFactor => ["SFFloat", 0.001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		angularVelocity => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		autoDamp => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		autoDisable => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		centerOfMass => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		disableAngularSpeed => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		disableLinearSpeed => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		disableTime => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		finiteRotationAxis => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fixed => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forces => ["MFVec3f",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry => ["MFNode",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		inertia => ["SFMatrix3f",[1,0,0,0,1,0,0,0,1],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		linearDampingFactor => ["SFFloat", 0.001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		linearVelocity => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass => ["SFFloat", 1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		massDensityModel => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		torques => ["MFVec3f",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		useFiniteRotation => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		useGlobalGravity => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_body => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		__old_angularVelocity => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_centerOfMass => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_finiteRotationAxis => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_linearVelocity => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", 0,"UNCA_NONE"],
		__old_position => ["SFVec3f", [0, 0, 0], "inputOutput", 0,"UNCA_NONE"],
		_geomIdentityTransform => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	], "X3DSFNode"),

	"RigidBodyCollection" => new VRML::NodeType("RigidBodyCollection", [
		set_contacts =>["MFNode",[],"inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		autoDisable => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bodies => ["MFNode",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		constantForceMix => ["SFFloat", .0001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		contactSurfaceThickness => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		disableAngularSpeed => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		disableLinearSpeed => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		disableTime => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		errorCorrection => ["SFFloat", 0.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		gravity => ["SFVec3f", [0, -9.8, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		iterations => ["SFInt32",10,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		joints => ["MFNode",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxCorrectionSpeed => ["SFFloat", -1.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		preferAccuracy => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		collider => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_world => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		#_space => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_group => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
	], "X3DChildNode"),

	"SingleAxisHingeJoint" => new VRML::NodeType("SingleAxisHingeJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis => ["SFVec3f", [0,0,1], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxAngle => ["SFFloat", "PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minAngle => ["SFFloat", "-PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopBounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		angleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_forceout => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_axis => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
	], "X3DRigidJointNode"),

	"SliderJoint" => new VRML::NodeType("SliderJoint", [
		axis => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxSeparation => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		minSeparation => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sliderForce => ["SFFloat", 0, "inputOutput", "( SPEC_X3D33)","UNCA_NONE"],
		stopBounce => ["SFFloat",  0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stopErrorCorrection => ["SFFloat",  1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		separation => ["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		separationRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_forceout => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__old_axis => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
	], "X3DRigidJointNode"),
	
	"UniversalJoint" => new VRML::NodeType("UniversalJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis1 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		axis2 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop1Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop1ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop2Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stop2ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body1Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		body2Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,"UNCA_NONE"],
		_forceout => ["SFInt32", 0, "initializeOnly", 0,"UNCA_NONE"],
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_axis1 => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_axis2 => ["SFVec3f", [0,0,0], "inputOutput", 0,"UNCA_NONE"],
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,"UNCA_NONE"],
	], "X3DRigidJointNode"),


	###################################################################################

	#	38.	Picking Component

	###################################################################################
	
	# A PickableGroup node is an X3DGroupingNode that contains children that are marked
	# as being of a given classification of picking types, as well as the ability to enable or disable picking of the children.

# DJTRACK_PICKSENSORS
	"PickableGroup" => new VRML::NodeType("PickableGroup", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickable => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		#FreeWRL__protoDef => ["SFInt32", "INT_ID_UNDEFINED", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # tell renderer that this is a proto...
		#FreeWRL_PROTOInterfaceNodes =>["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DGroupingNode"),

	# The PointPickSensor node tests one or more points in space as lying inside the provided target geometry.
	# For each point that lies inside the geometry, the point coordinate is returned in the pickedGeometry field
	# with the corresponding geometry inside which the point lies.
	# Because points represent an infinitely small location in space, the "CLOSEST" and "ALL_SORTED" sort orders
	# are defined to mean "ANY" and "ALL" respectively.

	"PointPickSensor" => new VRML::NodeType("PointPickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_X3D33)","UNCA_NONE"],
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
		pickedPoint => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		
		#DJTRACK
		_oldisActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldpickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldpickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_oldpickedPoint => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_intersectionType => ["SFString", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_sortOrder => ["SFString", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DSensorNode"),

	"LinePickSensor" => new VRML::NodeType("LinePickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
		pickedPoint => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickedNormal => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickedTextureCoordinate => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DSensorNode"),
	
	#38.4.4 PrimitivePickSensor
	"PrimitivePickSensor" => new VRML::NodeType("PrimitivePickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DSensorNode"),
	
	
	#38.4.5 VolumePickSensor
	"VolumePickSensor" => new VRML::NodeType("VolumePickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],
	],"X3DSensorNode"),
	
	###################################################################################

	#	39.	Followers Component

	###################################################################################
	
	# value_changed is the first field-type-sepcific field so that offsetof(,value_changed) will be generic for all chasers, and for all dampers
	"ColorChaser" => new VRML::NodeType("ColorChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_steptime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFColor", [0,0,0], "outputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFColor", [.8,.8,.8], "initializeOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFColor", [.8,.8,.8], "initializeOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFColor", [0,0,0], "inputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFColor", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_previousvalue => ["SFColor", [0,0,0], "initializeOnly", 0,"UNCA_NONE"],
		_destination => ["SFColor", [0,0,0], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChaserNode"),
	
	"ColorDamper" => new VRML::NodeType("ColorDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		tau  => ["SFTime", .3,"inputOutput", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFColor", [0,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFColor", [.8,.8,.8], "initializeOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFColor", [.8,.8,.8], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFColor", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFColor", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_input => ["SFColor", [0,0,0], "initializeOnly", 0,"UNCA_NONE"],
		
	],"X3DDamperNode"),
	
	"CoordinateChaser" => new VRML::NodeType("CoordinateChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_steptime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_previousvalue => ["MFVec3f", [[0,0,0]], "initializeOnly", 0,"UNCA_NONE"],
		_destination => ["MFVec3f", [[0,0,0]], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChaserNode"),
	
	"CoordinateDamper" => new VRML::NodeType("CoordinateDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_input => ["MFVec3f", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DDamperNode"),

	"OrientationChaser" => new VRML::NodeType("OrientationChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_steptime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFRotation", [0,1,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_previousvalue => ["SFRotation", [0,1, 0,0], "initializeOnly", 0,"UNCA_NONE"],
		_destination => ["SFRotation", [0,1,0,0], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChaserNode"),
	
	"OrientationDamper" => new VRML::NodeType("OrientationDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFRotation", [0,1,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_input => ["SFRotation", [0,1,0,0], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DDamperNode"),

	"PositionChaser" => new VRML::NodeType("PositionChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_steptime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_previousvalue => ["SFVec3f", [0,0,0], "initializeOnly", 0,"UNCA_NONE"],
		_destination => ["SFVec3f", [0,0,0], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChaserNode"),
	
	"PositionDamper" => new VRML::NodeType("PositionDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_input => ["SFVec3f", [0,0,0], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DDamperNode"),

	"PositionChaser2D" => new VRML::NodeType("PositionChaser2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_steptime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFVec2f", [0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_previousvalue => ["SFVec2f", [0,0], "initializeOnly", 0,"UNCA_NONE"],
		_destination => ["SFVec2f", [0,0], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChaserNode"),
	
	"PositionDamper2D" => new VRML::NodeType("PositionDamper2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFVec2f", [0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_input => ["SFVec2f", [0,0], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DDamperNode"),

	"ScalarChaser" => new VRML::NodeType("ScalarChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_steptime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFFloat", 0, "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_previousvalue => ["SFFloat", 0, "initializeOnly", 0,"UNCA_NONE"],
		_destination => ["SFFloat", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChaserNode"),
	
	"ScalarDamper" => new VRML::NodeType("ScalarDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["SFFloat", 0, "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_input => ["SFFloat", 0, "initializeOnly", 0,"UNCA_NONE"],
	],"X3DDamperNode"),

	"TexCoordChaser2D" => new VRML::NodeType("TexCoordChaser2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_steptime  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["MFVec2f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_previousvalue => ["MFVec2f", [[0,0]], "initializeOnly", 0,"UNCA_NONE"],
		_destination => ["MFVec2f", [[0,0]], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DChaserNode"),
	
	"TexCoordDamper2D" => new VRML::NodeType("TexCoordDamper2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,"UNCA_NONE"],
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,"UNCA_NONE"],
		value_changed => ["MFVec2f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialDestination => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		initialValue => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_destination => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		set_value => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_input => ["MFVec2f", [], "initializeOnly", 0,"UNCA_NONE"],
	],"X3DDamperNode"),


	###################################################################################

	#	40.	Particle Systems Component

	###################################################################################
	#40.4.1 BoundedPhysicsModel
	"BoundedPhysicsModel" => new VRML::NodeType("BoundedPhysicsModel", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DParticlePhysicsModelNode"),
	
	# 40.4.2 ConeEmitter
	"ConeEmitter" => new VRML::NodeType("ConeEmitter", [
		angle  => ["SFFloat", "PIF*.25","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position  => ["SFVec3f", [0,0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DParticleEmitterNode"),
	
	# 40.4.3 ExplosionEmitter
	"ExplosionEmitter" => new VRML::NodeType("ExplosionEmitter", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position  => ["SFVec3f", [0,0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DParticleEmitterNode"),
	
	# 40.4.4 ForcePhysicsModel
	"ForcePhysicsModel" => new VRML::NodeType("ForcePhysicsModel", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		force => ["SFVec3f", [0,-9.8,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DParticlePhysicsModelNode"),
	
	
	# 40.4.5 ParticleSystem
	"ParticleSystem" => new VRML::NodeType ("ParticleSystem", [
		# shared with Shape, keep in same order as Shape:
		appearance => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_shaderflags_base =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"], # shaders
		_shaderflags_effects =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"], # shaders
		_shaderflags_usershaders =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"], # shaders
		# particlesystem specific:
		createParticles  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		lifetimeVariation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		maxParticles  => ["SFInt32", 200,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		particleLifetime  => ["SFFloat", 5,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		particleSize => ["SFVec2f", [.02,.02], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive  => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorRamp => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorKey => ["MFFloat", [], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		emitter => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometryType => ["SFString", "QUAD", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		physics => ["MFNode", [], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoordRamp => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		texCoordKey => ["MFFloat", [], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_tris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_ttex => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_ltex => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_particles => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_lasttime => ["SFDouble", 0.0, "initializeOnly", 0,"UNCA_NONE"],
		_geometryType =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_remainder =>["SFFloat",0.0,"initializeOnly",0,"UNCA_NONE"],
	],"X3DShapeNode"),
	
	# 40.4.6 PointEmitter
	"PointEmitter" => new VRML::NodeType("PointEmitter", [
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position  => ["SFVec3f", [0,0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DParticleEmitterNode"),
	
	# 40.4.7 PolylineEmitter
	"PolylineEmitter" => new VRML::NodeType("PolylineEmitter", [
		set_coordIndex => ["MFInt32", [], "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],
		set_coordinate => ["SFInt32", 0, "inputOnly", "(SPEC_X3D32)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coordIndex => ["MFInt32", [-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_method =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_nseg =>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_segs => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		_portions => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
	],"X3DParticleEmitterNode"),
	
	# 40.4.8 SurfaceEmitter
	"SurfaceEmitter" => new VRML::NodeType("SurfaceEmitter", [
		set_coordIndex => ["MFInt32", [], "inputOnly", "( SPEC_X3D33)","UNCA_NONE"],
		set_coordinate => ["SFInt32", 0, "inputOnly", "(SPEC_X3D32)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coordIndex => ["MFInt32", [-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surface => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_ifs => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
	],"X3DParticleEmitterNode"),
	
	# 40.4.9 VolumeEmitter
	"VolumeEmitter" => new VRML::NodeType("VolumeEmitter", [
		set_coordIndex => ["MFInt32", [], "inputOnly", "( SPEC_X3D33)","UNCA_NONE"],
		set_coordinate => ["SFInt32", 0, "inputOnly", "(SPEC_X3D32)","UNCA_NONE"],
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coordIndex => ["MFInt32", [-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		internal  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_ifs => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
	],"X3DParticleEmitterNode"),
	
	# 40.4.10 WindPhysicsModel
	"WindPhysicsModel" => new VRML::NodeType("WindPhysicsModel", [
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		gustiness => ["SFFloat", .1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		speed  => ["SFFloat", .1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		turbulence  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_frameSpeed =>["SFFloat",0.0,"initializeOnly",0,"UNCA_NONE"],
	],"X3DParticlePhysicsModelNode"),
	

	###################################################################################

	#	41.	Volume Rendering Component

	###################################################################################
	# LEVEL 1
	
	"OpacityMapVolumeStyle" => new VRML::NodeType("OpacityMapVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transferFunction => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	"VolumeData" => new VRML::NodeType("VolumeData", [
		dimensions  => ["SFVec3f", [1,1,1],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_boxtris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		renderStyle => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DVolumeDataNode"),


	#level 2
	# BoundaryEnhancementVolumeStyle	All fields fully supported.
	"BoundaryEnhancementVolumeStyle" => new VRML::NodeType("BoundaryEnhancementVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		boundaryOpacity => ["SFFloat", .9,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		opacityFactor => ["SFFloat", 2.0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		retainedOpacity => ["SFFloat", .2,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	# ComposedVolumeStyle	ordered field is always treated as FALSE. All other fields fully supported.
	"ComposedVolumeStyle" => new VRML::NodeType("ComposedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	# EdgeEnhancementVolumeStyle	All fields fully supported.
	"EdgeEnhancementVolumeStyle" => new VRML::NodeType("EdgeEnhancementVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		edgeColor=>["SFColorRGBA",[0,0,0,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		gradientThreshold => ["SFFloat", .4,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	# IsoSurfaceVolumeData	All fields fully supported.
	"IsoSurfaceVolumeData" => new VRML::NodeType("IsoSurfaceVolumeData", [
		dimensions  => ["SFVec3f", [1,1,1],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_boxtris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		contourStepSize => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		gradients => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceTolerance => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceValues => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"], # see note top of file
	],"X3DVolumeDataNode"),
	
	# see level1: OpacityMapVolumeStyle	All fields fully supported. 3D transfer functions shall be supported.
	# ProjectionVolumeStyle	All fields fully supported
	"ProjectionVolumeStyle" => new VRML::NodeType("ProjectionVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		intensityThreshold => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		type => ["SFString", "MAX", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_type => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],		
	],"X3DComposableVolumeRenderStyleNode"),
	
	# SegmentedVolumeData	All fields fully supported.
	"SegmentedVolumeData" => new VRML::NodeType("SegmentedVolumeData", [
		dimensions  => ["SFVec3f", [1,1,1],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_boxtris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,"UNCA_NONE"],
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		segmentEnabled => ["MFBool",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],  # see note top of file
		segmentIdentifiers => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DVolumeDataNode"),
	
	# SilhouetteEnhancementVolumeStyle	All fields fully supported.
	"SilhouetteEnhancementVolumeStyle" => new VRML::NodeType("SilhouetteEnhancementVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		silhouetteBoundaryOpacity => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		silhouetteRetainedOpacity => ["SFFloat", 1,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		silhouetteSharpness => ["SFFloat", .5,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	
	# ToneMappedVolumeStyle	All fields fully supported.
	"ToneMappedVolumeStyle" => new VRML::NodeType("ToneMappedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		coolColor=>["SFColorRGBA",[0,0,1,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		warmColor=>["SFColorRGBA",[1,1,0,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	#level 3
	# BlendedVolumeStyle	All fields fully supported.
	"BlendedVolumeStyle" => new VRML::NodeType("BlendedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		renderStyle => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weightConstant1 => ["SFFloat", 0.5,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weightConstant2 => ["SFFloat", 0.5,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weightFunction1 => ["SFString", "CONSTANT", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weightFunction2 => ["SFString", "CONSTANT", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weightTransferFunction1 => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		weightTransferFunction2 => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_fbohandles => ["MFInt32",[0,0,0],"initializeOnly",0,"UNCA_NONE"],
		_weightFunction1 => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
		_weightFunction2 => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	# CartoonVolumeStyle	All fields fully supported.
	"CartoonVolumeStyle" => new VRML::NodeType("CartoonVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		orthogonalColor=>["SFColorRGBA",[1,1,1,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		parallelColor=>["SFColorRGBA",[0,0,0,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		colorSteps => ["SFInt32", 4, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	# CompositeVolumeStyle	All fields fully supported.
	"CompositeVolumeStyle" => new VRML::NodeType("CompositeVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		#the other renderStyles are SF, this one MF
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),
	
	# ShadedVolumeStyle	All fields fully supported except shadows. Shadows supported with at least Phong shading.
	#level 4
	# ShadedVolumeStyle	All fields fully supported with at least Phong shading and  Henyey-Greenstein phase function. Shadows fully supported.
	"ShadedVolumeStyle" => new VRML::NodeType("ShadedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		lighting  => ["SFBool", "FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		shadows => ["SFBool", "FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		material => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		phaseFunction => ["SFString", "Henyey-Greenstein", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		_phaseFunction => ["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],
	],"X3DComposableVolumeRenderStyleNode"),


	###################################################################################

	# Augmented Reality - not in specs, proposed:
	# http://www.web3d.org/wiki/index.php?title=AR_Proposal_Public_Review
	
	###################################################################################

	"BackdropBackground" => new VRML::NodeType("BackdropBackground", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transparency => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFColor", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__texture => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"],
		__VBO=>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],  # Vertex Buffer Object, if required.
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DBackgroundNode"),
	
	"ImageBackdropBackground" => new VRML::NodeType("ImageBackdropBackground", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		transparency => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		color => ["SFColor", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		__texture => ["SFInt32", 0, "inputOutput", 0,"UNCA_NONE"],
		__VBO=>["SFInt32",0,"initializeOnly",0,"UNCA_NONE"],  # Vertex Buffer Object, if required.
		image => ["SFImage", "0, 0, 0", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DBackgroundNode"),

	"CalibratedCameraSensor" => new VRML::NodeType("CalibratedCameraSensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		image => ["SFImage", "0, 0, 0", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		focalPoint => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fieldOfView => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		fovMode => ["SFString", "", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		aspectRatio => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DSensorNode"),
	
	"TrackingSensor" => new VRML::NodeType("TrackingSensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		position => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		rotation => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isPositionAvailable => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		isRotationAvailable => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
	],"X3DSensorNode"),


	# Metadata nodes ...

	###################################################################################

	#used mainly for (pre-2014 era text-based PROTOs attached to Group nodes aka TROTO) PROTO invocation parameters
	#(2014+ era: switched to binary PROTOs (aka Brotos) with their own (not Group) node, which uses routing to go from 
	#  BrotoInterface to BrotoBody nodes - don't need the following now, or the __protoDEF thing in Group, 
	#   except to compile left-over code)
	"MetadataSFFloat" => new VRML::NodeType("MetadataSFFloat", [
		value => ["SFFloat",0.0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFFloat",0.0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFFloat",0.0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFFloat" => new VRML::NodeType("MetadataMFFloat", [
		value => ["MFFloat",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFFloat",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFFloat",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFRotation" => new VRML::NodeType("MetadataSFRotation", [
		value => ["SFRotation",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFRotation",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFRotation",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFRotation" => new VRML::NodeType("MetadataMFRotation", [
		value => ["MFRotation",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFRotation",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFRotation",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec3f" => new VRML::NodeType("MetadataSFVec3f", [
		value => ["SFVec3f",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFVec3f",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFVec3f",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec3f" => new VRML::NodeType("MetadataMFVec3f", [
		value => ["MFVec3f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFVec3f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFVec3f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFBool" => new VRML::NodeType("MetadataSFBool", [
		value => ["SFBool","FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFBool","FALSE","outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFBool","FALSE","inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFBool" => new VRML::NodeType("MetadataMFBool", [
		value => ["MFBool",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFBool",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFBool",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFInt32" => new VRML::NodeType("MetadataSFInt32", [
		value => ["SFInt32",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFInt32",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFInt32",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFInt32" => new VRML::NodeType("MetadataMFInt32", [
		value => ["MFInt32",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFInt32",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFInt32",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFNode" => new VRML::NodeType("MetadataSFNode", [
		value => ["SFNode",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFNode",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFNode",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFNode" => new VRML::NodeType("MetadataMFNode", [
		value => ["MFNode",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFNode",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFNode",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFColor" => new VRML::NodeType("MetadataSFColor", [
		value => ["SFColor",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFColor",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFColor",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFColor" => new VRML::NodeType("MetadataMFColor", [
		value => ["MFColor",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFColor",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFColor",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFColorRGBA" => new VRML::NodeType("MetadataSFColorRGBA", [
		value => ["SFColorRGBA",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFColorRGBA",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFColorRGBA",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFColorRGBA" => new VRML::NodeType("MetadataMFColorRGBA", [
		value => ["MFColorRGBA",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFColorRGBA",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFColorRGBA",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFTime" => new VRML::NodeType("MetadataSFTime", [
		value => ["SFTime",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFTime",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFTime",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFTime" => new VRML::NodeType("MetadataMFTime", [
		value => ["MFTime",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFTime",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFTime",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFString" => new VRML::NodeType("MetadataSFString", [
		value => ["SFString","","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFString","","outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFString","","inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFString" => new VRML::NodeType("MetadataMFString", [
		value => ["MFString",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFString",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFString",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec2f" => new VRML::NodeType("MetadataSFVec2f", [
		value => ["SFVec2f",[0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFVec2f",[0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFVec2f",[0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec2f" => new VRML::NodeType("MetadataMFVec2f", [
		value => ["MFVec2f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFVec2f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFVec2f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFImage" => new VRML::NodeType("MetadataSFImage", [
		value => ["SFImage",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFImage",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFImage",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec3d" => new VRML::NodeType("MetadataSFVec3d", [
		value => ["SFVec3d",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFVec3d",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFVec3d",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec3d" => new VRML::NodeType("MetadataMFVec3d", [
		value => ["MFVec3d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFVec3d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFVec3d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFDouble" => new VRML::NodeType("MetadataSFDouble", [
		value => ["SFDouble",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFDouble",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFDouble",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFDouble" => new VRML::NodeType("MetadataMFDouble", [
		value => ["MFDouble",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFDouble",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFDouble",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix3f" => new VRML::NodeType("MetadataSFMatrix3f", [
		value => ["SFMatrix3f",[0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFMatrix3f",[0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFMatrix3f",[0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix3f" => new VRML::NodeType("MetadataMFMatrix3f", [
		value => ["MFMatrix3f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFMatrix3f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFMatrix3f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix3d" => new VRML::NodeType("MetadataSFMatrix3d", [
		value => ["SFMatrix3d",[0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFMatrix3d",[0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFMatrix3d",[0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix3d" => new VRML::NodeType("MetadataMFMatrix3d", [
		value => ["MFMatrix3d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFMatrix3d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFMatrix3d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix4f" => new VRML::NodeType("MetadataSFMatrix4f", [
		value => ["SFMatrix4f",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFMatrix4f",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFMatrix4f",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix4f" => new VRML::NodeType("MetadataMFMatrix4f", [
		value => ["MFMatrix4f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFMatrix4f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFMatrix4f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix4d" => new VRML::NodeType("MetadataSFMatrix4d", [
		value => ["SFMatrix4d",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFMatrix4d",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFMatrix4d",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix4d" => new VRML::NodeType("MetadataMFMatrix4d", [
		value => ["MFMatrix4d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFMatrix4d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFMatrix4d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec2d" => new VRML::NodeType("MetadataSFVec2d", [
		value => ["SFVec2d",[0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFVec2d",[0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFVec2d",[0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec2d" => new VRML::NodeType("MetadataMFVec2d", [
		value => ["MFVec2d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFVec2d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFVec2d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec4f" => new VRML::NodeType("MetadataSFVec4f", [
		value => ["SFVec4f",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFVec4f",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFVec4f",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec4f" => new VRML::NodeType("MetadataMFVec4f", [
		value => ["MFVec4f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFVec4f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFVec4f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec4d" => new VRML::NodeType("MetadataSFVec4d", [
		value => ["SFVec4d",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["SFVec4d",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["SFVec4d",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec4d" => new VRML::NodeType("MetadataMFVec4d", [
		value => ["MFVec4d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		valueChanged=>["MFVec4d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		setValue =>["MFVec4d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		tickTime=>["SFTime",0,"inputOnly",0,"UNCA_NONE"],
	], "X3DChildNode"),




	###################################################################################

	# testing...

	###################################################################################
#
# Experimental node: OSC_Sensor
#
# Theory:
# You define one or more OSC_Sensor node in your VRML file,
# and route 'gotEvent' to a JavaScript node which triggers
# when incoming data is received. Vague plans for a OSC_transmitter.
#
# Caveat: Only UDP is supported because that seems to be a hole in liblo.
# *It defines  lo_server_thread_new_with_proto but does not seem to implement it.*
#
# See sample WRL: freewrl/tests/18-OSC-1.wrl
#
# listenfor: typical OSC data spec, for example iii
# filter: typical OSC filter, for example /alpha/beta/gamma
#
# handler: You can choose to write your own handler in src/lib/scenegraph/OSCcallbacks.c
# Uses: You may choose to take 3 incoming delta values and turn it into a vector.
#	You may want to examine the values and turn a stream of packets into a single gesture
# 2 handlers are supplied to use 'as is' or to use as a template for your own code:
# nullOSC_handler - just swallows the callback. This is invoked if handler is undefined (or "")
# defaultOSC_handler - just puts the data into the FIFOs; invoked if handler is defined as "default"
#
# Incoming values are placed into a set of FIFOs. So, if the external agent sent 3 deltas values,
# all 3 values are placed into the FIFO. So, the dummy values intVal, strVal and fltVal are there
# merely so that the JavaScript has something to talk about. The actaul Javascript utility routines
# have been modified to look at FIFOsize. If FIFOsize > 0, then instead of doing a memcpy the
# utility routines retrieve a single value out of the respective FIFO
#
	"OSC_Sensor" => new VRML::NodeType("OSC_Sensor", [

		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		protocol => ["SFString", "UDP", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		listenfor => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		port => ["SFInt32", 7000, "inputOutput", 0,"UNCA_NONE"],
		filter => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		handler => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		talksTo => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		FIFOsize  => ["SFInt32", 64, "inputOutput",, 0,"UNCA_NONE"],
		int32Inp => ["SFInt32", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		floatInp => ["SFFloat", 0.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		stringInp => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],
		gotEvents => ["SFInt32", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],

		_talkToNodes => ["MFNode", [], "inputOutput", 0,"UNCA_NONE"],
		_status => ["SFInt32", -1, "inputOutput", 0,"UNCA_NONE"],
		_int32InpFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_floatInpFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_stringInpFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_int32OutFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_floatOutFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		_stringOutFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,"UNCA_NONE"],
		__oldmetadata => ["SFNode", 0, "inputOutput", 0,"UNCA_NONE"], # see code for event macro

	],"X3DNetworkSensorNode"),




);


1;

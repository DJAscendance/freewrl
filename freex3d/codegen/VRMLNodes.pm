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
			
			$t = $field[4];
			if (!defined $t) {
				die("Missing field or event type $type X3DNodeType $X3DNodeType for $fname in $name");
			}
			$this->{Unca}{$fname} = $t;

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
		info => ["MFString", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff the # f f end is so yoou can add another attribute with replace all
		title => ["SFString", "", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),

	"Proto" => new VRML::NodeType("Proto", [
		# sept 2014: keep Inline the same as Proto, so one can be cast to the other, unless/until executionContext is extracted from both
		__children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__protoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__externProtoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__nodes => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__subcontexts => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__GC => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__protoDef => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff #user fields
		__protoFlags => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__prototype => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff #first node in protobody
		__parentProto => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff #first node in protobody
		__ROUTES => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__EXPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__IMPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__DEFnames => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__IS => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__scripts => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__oldurl => ["MFString", [], "initializeOnly", 0,0],#ff
		__afterPound => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__typename => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		load => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__oldload => ["SFBool", "FALSE", "initializeOnly", 0,0],#ff
		__unitlengthfactor => ["SFDouble", 1.0, "initializeOnly", 0,0],#ff
		__specversion => ["SFInt32",0,"initializeOnly",0,0],#ff
	],"X3DProtoInstance"),

	"MetadataBoolean" => new VRML::NodeType("MetadataBoolean", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			value => ["MFBool",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff  # see note top of file
	], "X3DChildNode"),

	"MetadataInteger" => new VRML::NodeType("MetadataInteger", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			value => ["MFInt32",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff  # see note top of file
	], "X3DChildNode"),

	"MetadataDouble" => new VRML::NodeType("MetadataDouble", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff# see note top of file:
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			value => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
	], "X3DChildNode"),

	"MetadataFloat" => new VRML::NodeType("MetadataFloat", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			value => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
	], "X3DChildNode"),

	"MetadataString" => new VRML::NodeType("MetadataString", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			value => ["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
	], "X3DChildNode"),

	"MetadataSet" => new VRML::NodeType("MetadataSet", [
			metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			name => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
			reference => ["SFString","","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
			value => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
	], "X3DChildNode"),

	###################################################################################

	# Chapter 8:		Time Component

	###################################################################################

	"TimeSensor" => new VRML::NodeType("TimeSensor", [
		cycleInterval => ["SFTime", 1, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		loop => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pauseTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		resumeTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		startTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stopTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		cycleTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		elapsedTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fraction_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isPaused => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		time => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# time that we were initialized at
		__inittime => ["SFTime", 0, "initializeOnly", 0,0],#ff
		# cycleTimer flag.
		__ctflag =>["SFTime", 10, "inputOutput", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
		__lasttime => ["SFTime", 0, "initializeOnly", 0,0],#ff
	],"X3DSensorNode"),

	###################################################################################

	# Chapter 9:		Networking Component

	###################################################################################

	"Anchor" => new VRML::NodeType("Anchor", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		parameter => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff

		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DGroupingNode"),


	"Inline" => new VRML::NodeType("Inline", [
		# sept 2014: keep Inline the same as Proto, so one can be cast to the other, unless/until executionContext is extracted from both
		__children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__protoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__externProtoDeclares => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__nodes => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__subcontexts => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__GC => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__protoDef => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff #user fields
		__protoFlags => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__prototype => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff #first node in protobody
		__parentProto => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff #first node in protobody
		__ROUTES => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__EXPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__IMPORTS => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__DEFnames => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__IS => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__scripts => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__oldurl => ["MFString", [], "initializeOnly", 0,0],#ff
		__afterPound => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__typename => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		load => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__oldload => ["SFBool", "FALSE", "initializeOnly", 0,0],#ff
		__unitlengthfactor => ["SFDouble", 1.0, "initializeOnly", 0,0],#ff
		__specversion => ["SFInt32",0,"initializeOnly",0,0],#ff
		
		# load => ["SFBool", "TRUE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

                # __children => ["MFNode", [], "inputOutput", 0,0],#ff
		# __loadstatus =>["SFInt32",0,"initializeOnly", 0],
		# _parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		 # __loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	],"X3DNetworkSensorNode"),

	"LoadSensor" => new VRML::NodeType("LoadSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		timeOut  => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		watchList => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isLoaded  => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		loadTime  => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		progress  => ["SFFloat",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		__loading => ["SFBool", "TRUE","initializeOnly", 0,0],#ff		# current internal status
		__finishedloading => ["SFBool", "TRUE","initializeOnly", 0,0],#ff	# current internal status
		__StartLoadTime => ["SFTime",0,"outputOnly", 0,0],#ff # time we started loading...
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DNetworkSensorNode"),


	###################################################################################

	# Chapter 10:		Grouping Component

	###################################################################################

	"Group" => new VRML::NodeType("Group", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	],"X3DGroupingNode"),

	"StaticGroup" => new VRML::NodeType("StaticGroup", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		__transparency => ["SFInt32", -1, "initializeOnly", 0,0],#ff # display list for transparencies
		__solid => ["SFInt32", -1, "initializeOnly", 0,0],#ff	 # display list for solid geoms.
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	],"X3DGroupingNode"),

	"Switch" => new VRML::NodeType("Switch", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		choice => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30)","UNCA_NONE"],#ff		# VRML nodes....
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff		# X3D nodes....
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		whichChoice => ["SFInt32", -1, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff

		__isX3D => ["SFBool", "(inputFileVersion[0]==3)" , "initializeOnly", 0,0],#ff # "TRUE" for X3D V3.x files
	],"X3DGroupingNode"),

	"Transform" => new VRML::NodeType ("Transform", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff

		# fields for reducing redundant calls
		__do_center => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_scaleO => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_scale => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_anything => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	],"X3DGroupingNode"),


	###################################################################################

	# Chapter 11:		Rendering Component

	###################################################################################

	"ClipPlane" => new VRML::NodeType("ClipPlane", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		plane => ["SFVec4f", [0, 1, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_PLANE"],#ff

	],"X3DChildNode"),

	"Color" => new VRML::NodeType("Color", [
		color => ["MFColor", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

	],"X3DColorNode"),

	"ColorRGBA" => new VRML::NodeType("ColorRGBA", [
		color => ["MFColorRGBA", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

	],"X3DColorNode"),

	"Coordinate" => new VRML::NodeType("Coordinate", [
		point => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

	],"X3DCoordinateNode"),

	"IndexedLineSet" => new VRML::NodeType("IndexedLineSet", [
		set_colorIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_coordIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coordIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__vertArr  =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__vertIndx  =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__xcolours  =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__vertices  =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__vertexCount =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__segCount =>["SFInt32",0,"initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"IndexedTriangleFanSet" => new VRML::NodeType("IndexedTriangleFanSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		index => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"IndexedTriangleSet" => new VRML::NodeType("IndexedTriangleSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		index => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"IndexedTriangleStripSet" => new VRML::NodeType("IndexedTriangleStripSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		index => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"LineSet" => new VRML::NodeType("LineSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vertexCount => ["MFInt32",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__vertArr  =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__vertIndx  =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__segCount =>["SFInt32",0,"initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"Normal" => new VRML::NodeType("Normal", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vector => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DNormalNode"),

	"PointSet" => new VRML::NodeType("PointSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_pointsVBO =>["SFInt32", 0, "initializeOnly", 0,0],#ff
		_coloursVBO =>["SFInt32", 0, "initializeOnly", 0,0],#ff
		_npoints =>["SFInt32", 0, "initializeOnly", 0,0],#ff
		_colourSize =>["SFInt32", 0, "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"TriangleFanSet" => new VRML::NodeType("TriangleFanSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fanCount => ["MFInt32", [3], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"TriangleStripSet" => new VRML::NodeType("TriangleStripSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stripCount => ["MFInt32", [], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff

	],"X3DGeometryNode"),

	"TriangleSet" => new VRML::NodeType("TriangleSet", [
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),


	###################################################################################

#	Chapter 12:		Shape Component

	###################################################################################

	"Appearance" => new VRML::NodeType ("Appearance", [
		fillProperties => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		lineProperties => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		material => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		shaders => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		effects => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texture => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureTransform => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DAppearanceNode"),

	"FillProperties" => new VRML::NodeType ("FillProperties", [
		filled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hatchColor => ["SFColor", [1,1,1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hatched => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hatchStyle => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_enabled =>["SFBool", "TRUE", "inputOutput",0,0],#ff # literally, is this thing used or not?
		_hatchScale =>["SFVec2f", [0.1,0.1], "inputOutput",0,0],#ff # the rate of the lines, 0.1 = 10 lines/meter
	],"X3DAppearanceChildNode"),

	"LineProperties" => new VRML::NodeType ("LineProperties", [
		applied => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		linetype => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		linewidthScaleFactor => ["SFFloat", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DAppearanceChildNode"),

	"Material" => new VRML::NodeType ("Material", [
		ambientIntensity => ["SFFloat", 0.2, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		diffuseColor => ["SFColor", [0.8, 0.8, 0.8], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		emissiveColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		shininess => ["SFFloat", 0.2, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		specularColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transparency => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_verifiedColor => ["MFFloat",[
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0],"initializeOnly",0,0],#ff # for making materials shader-friendly
	],"X3DMaterialNode"),

	"Shape" => new VRML::NodeType ("Shape", [
		# shared with particlesystem, keep in same order as particlesystem:
		appearance => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_shaderflags_base =>["SFInt32",0,"initializeOnly",0,0],#ff # shaders
		_shaderflags_effects =>["SFInt32",0,"initializeOnly",0,0],#ff # shaders
		_shaderflags_usershaders =>["SFInt32",0,"initializeOnly",0,0],#ff # shaders
		# shape-specific:
		__visible =>["SFInt32",0,"initializeOnly", 0,0],#ff # for Occlusion tests.
		__occludeCheckCount =>["SFInt32",-1,"initializeOnly", 0,0],#ff # for Occlusion tests.
		__Samples =>["SFInt32",-1,"initializeOnly", 0,0],#ff		# Occlude samples from last pass

	],"X3DBoundedObject"),

	"TwoSidedMaterial" => new VRML::NodeType ("TwoSidedMaterial", [
		ambientIntensity => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		backAmbientIntensity => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		backDiffuseColor => ["SFColor", [0.8, 0.8, 0.8], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		backEmissiveColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		backShininess => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		backSpecularColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		backTransparency => ["SFFloat", 0, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		diffuseColor => ["SFColor", [0.8, 0.8, 0.8], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		emissiveColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		shininess => ["SFFloat", 0.2, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		separateBackColor =>["SFBool","FALSE","inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		specularColor => ["SFColor", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		transparency => ["SFFloat", 0, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		_verifiedFrontColor => ["MFFloat",[
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0],"initializeOnly",0,0],#ff # for making materials shader-friendly
		_verifiedBackColor => ["MFFloat",[
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0, 0.0, 0.0, 0.0,
			0.0],"initializeOnly",0,0],#ff # for making materials shader-friendly
	],"X3DMaterialNode"),



	###################################################################################

	# Chapter 13:		Geometry3D Component

	###################################################################################

	"Box" => new VRML::NodeType("Box", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["SFVec3f", [2, 2, 2], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__points  =>["MFVec3f",[],"initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"Cone" => new VRML::NodeType ("Cone", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bottom => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		bottomRadius => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		height => ["SFFloat", 2.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		side => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		 __sidepoints =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		 __botpoints =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		 __normals =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		__coneVBO =>["SFInt32",0,"initializeOnly", 0,0],#ff
		__coneTriangles =>["SFInt32",0,"initializeOnly",0,0],#ff
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"Cylinder" => new VRML::NodeType ("Cylinder", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bottom => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		height => ["SFFloat", 2.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		side => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		top => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		 __points =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		 __normals =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		__cylinderVBO =>["SFInt32",0,"initializeOnly",0,0],#ff
		__cylinderTriangles =>["SFInt32",0,"initializeOnly",0,0],#ff
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"ElevationGrid" => new VRML::NodeType("ElevationGrid", [
		set_height => ["MFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		creaseAngle => ["SFFloat", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		height => ["MFFloat", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		xDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		xSpacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		zDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		zSpacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"Extrusion" => new VRML::NodeType("Extrusion", [
		set_crossSection => ["MFVec2f", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_orientation => ["MFRotation", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_scale => ["MFVec2f", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_spine => ["MFVec3f", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		beginCap => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		convex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		creaseAngle => ["SFFloat", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		crossSection => ["MFVec2f", [[1, 1],[1, -1],[-1, -1],
						   [-1, 1],[1, 1]], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		endCap => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orientation => ["MFRotation", [[0, 0, 1, 0]],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff # see note top of file
		scale => ["MFVec2f", [[1, 1]], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		spine => ["MFVec3f", [[0, 0, 0],[0, 1, 0]], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DGeometryNode"),

	"IndexedFaceSet" => new VRML::NodeType("IndexedFaceSet", [
		set_colorIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_coordIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_normalIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_texCoordIndex => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attrib	=> ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		convex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coordIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		creaseAngle => ["SFFloat", 0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		normalIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoordIndex => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DGeometryNode"),

	"Sphere" => new VRML::NodeType("Sphere", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__points =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		_sideVBO =>["SFInt32", 0, "initializeOnly", 0,0],#ff
		__SphereIndxVBO =>["SFInt32", 0, "initializeOnly", 0,0],#ff
		__pindices => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__wireindicesVBO =>["SFInt32", 0, "initializeOnly", 0,0],#ff
 	],"X3DGeometryNode"),

	"Teapot" => new VRML::NodeType("Teapot", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__ifsnode => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
 	],"X3DGeometryNode"),



	###################################################################################

	#	Chapter 14:	Geometry 2D Component

	###################################################################################

	"Arc2D" => new VRML::NodeType("Arc2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		endAngle => ["SFFloat", 1.5707, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		startAngle => ["SFFloat", 0.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		__points  =>["MFVec2f",[],"initializeOnly", 0,0],#ff
		__numPoints =>["SFInt32",0,"initializeOnly", 0,0],#ff
 	],"X3DGeometryNode"),

	"ArcClose2D" => new VRML::NodeType("ArcClose2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		closureType => ["SFString","PIE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		endAngle => ["SFFloat", 1.5707, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		startAngle => ["SFFloat", 0.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		__points  =>["MFVec2f",[],"initializeOnly", 0,0],#ff
		__texCoords  =>["MFVec2f",[],"initializeOnly", 0,0],#ff
		__numPoints =>["SFInt32",0,"initializeOnly", 0,0],#ff
		__simpleDisk => ["SFBool", "TRUE","initializeOnly", 0,0],#ff
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
 	],"X3DGeometryNode"),


	"Circle2D" => new VRML::NodeType("Circle2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radius => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		__points  =>["MFVec2f",[],"initializeOnly", 0,0],#ff
		__numPoints =>["SFInt32",0,"initializeOnly", 0,0],#ff
 	],"X3DGeometryNode"),

	"Disk2D" => new VRML::NodeType("Disk2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		innerRadius => ["SFFloat", 0.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		outerRadius => ["SFFloat", 1.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__points  =>["MFVec2f",[],"initializeOnly", 0,0],#ff
		__texCoords  =>["MFVec2f",[],"initializeOnly", 0,0],#ff
		__numPoints =>["SFInt32",0,"initializeOnly", 0,0],#ff
		__simpleDisk => ["SFBool", "TRUE","initializeOnly", 0,0],#ff
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"Polyline2D" => new VRML::NodeType("Polyline2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		lineSegments => ["MFVec2f", [], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
 	],"X3DGeometryNode"),

	"Polypoint2D" => new VRML::NodeType("Polypoint2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		point => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
 	],"X3DGeometryNode"),

	"Rectangle2D" => new VRML::NodeType("Rectangle2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["SFVec2f", [2.0, 2.0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__points  =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		__numPoints =>["SFInt32",0,"initializeOnly", 0,0],#ff
 	],"X3DGeometryNode"),


	"TriangleSet2D" => new VRML::NodeType("TriangleSet2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vertices => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		solid => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__texCoords  =>["MFVec2f",[],"initializeOnly", 0,0],#ff
		__wireindices => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
 	],"X3DGeometryNode"),

	###################################################################################

	#	Chapter 15:		Text Component

	###################################################################################

	"Text" => new VRML::NodeType ("Text", [
		fontStyle => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		length => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		maxExtent => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		string => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		lineBounds => ["MFVec2f",[],"outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		origin => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textBounds => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_isScreen => ["SFInt32", 0, "inputOutput", 0,0],#ff # > 0 for screenfont
		_screendata => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff # screentext rowvec
	],"X3DTextNode"),

	"FontStyle" => new VRML::NodeType("FontStyle", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		family => ["MFString", ["SERIF"], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		horizontal => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		justify => ["MFString", ["BEGIN"], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		language => ["SFString", "", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		leftToRight => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		spacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		style => ["SFString", "PLAIN", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		topToBottom => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DFontStyleNode"),

	###################################################################################

	#	Chapter 16:		Sound Component

	###################################################################################

	"AudioClip" => new VRML::NodeType("AudioClip", [
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		loop =>	["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pauseTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pitch => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		resumeTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		startTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stopTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration_changed => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		elapsedTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isPaused => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,0],#ff
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		# internal sequence number, openal buffer number
		__sourceNumber => ["SFInt32", -1, "initializeOnly", 0,0],#ff
		# time that we were initialized at
		__inittime => ["SFTime", 0, "initializeOnly", 0,0],#ff
		__lasttime => ["SFTime", 0, "initializeOnly", 0,0],#ff
		# local name, as received on system
		# old audio __localFileName => ["FreeWRLPTR", 0,"initializeOnly", 0,0],#ff
	],"X3DSoundSourceNode"),

	"Sound" => new VRML::NodeType("Sound", [
		direction => ["SFVec3f", [0, 0, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		location => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		maxBack => ["SFFloat", 10.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		maxFront => ["SFFloat", 10.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		minBack => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		minFront => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		priority => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		source => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		spatialize => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# openal sound source number
		__sourceNumber => ["SFInt32", -1, "initializeOnly", 0,0],#ff
		__lastlocation => ["SFVec3f", [0, 0, 0], "initializeOnly",0,0],#ff
		__lasttime => ["SFTime", 0, "initializeOnly", 0,0],#ff
	],"X3DSoundSourceNode"),


	###################################################################################

	# Chapter 17:		Lighting Component

	###################################################################################

	"DirectionalLight" => new VRML::NodeType("DirectionalLight", [
		ambientIntensity => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		direction => ["SFVec3f", [0, 0, -1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		global => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		on => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_dir =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
		_col =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
		_amb =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
	],"X3DLightNode"),

	"PointLight" => new VRML::NodeType("PointLight", [
		ambientIntensity => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attenuation => ["SFVec3f", [1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		global => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		location => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		on => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radius => ["SFFloat", 100.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		_loc =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
		_col =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
		_amb =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
	],"X3DLightNode"),

	"SpotLight" => new VRML::NodeType("SpotLight", [
		ambientIntensity => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attenuation => ["SFVec3f", [1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		beamWidth => ["SFFloat", 1.570796, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		cutOffAngle => ["SFFloat", 0.785398, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		direction => ["SFVec3f", [0, 0, -1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		global => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intensity => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		location => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		on => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radius => ["SFFloat", 100.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		_dir =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
		_loc =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
		_col =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
		_amb =>["SFVec4f",[0,0,0,0],"initializeOnly",0,0],#ff
	],"X3DLightNode"),

	###################################################################################

	#	Chapter18:	Texturing Component

	###################################################################################

	"ImageTexture" => new VRML::NodeType("ImageTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatS => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatT => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DTextureNode"),

	"MovieTexture" => new VRML::NodeType ("MovieTexture", [
		#SoundSource / AudioClip compatible section, keep in same order as AudioClip
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		loop => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pauseTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pitch => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		resumeTime => ["SFTime",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		startTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stopTime => ["SFTime", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [""], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration_changed => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		elapsedTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isPaused => ["SFBool","FALSE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,0],#ff
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		# internal sequence number
		__sourceNumber => ["SFInt32", -1, "initializeOnly", 0,0],#ff
		# time that we were initialized at
		__inittime => ["SFTime", 0, "initializeOnly", 0,0],#ff
		__lasttime => ["SFTime", 0, "initializeOnly", 0,0],#ff
		#Texture2D and Movie section
		repeatS => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatT => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		speed => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		 __frac => ["SFFloat", 0.0, "initializeOnly", 0,0],#ff		
		 # which texture number is used
		 __ctex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		 # lowest frame
		 __lowest => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		 # highest frame
		 __highest => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		 __fw_movie  => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	],"X3DTextureNode"),


	"MultiTexture" => new VRML::NodeType("MultiTexture", [
		alpha =>["SFFloat", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color =>["SFColor",[1,1,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		function =>["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mode =>["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		source =>["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texture=>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__xparams => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	],"X3DTextureNode"),

	"MultiTextureCoordinate" => new VRML::NodeType("MultiTextureCoordinate", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord =>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureCoordinateNode"),

	"MultiTextureTransform" => new VRML::NodeType("MultiTextureTransform", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureTransform=>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureTransformNode"),

	"PixelTexture" => new VRML::NodeType("PixelTexture", [
		image => ["SFImage", "0, 0, 0", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatS => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatT => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
	],"X3DTextureNode"),

	"TextureCoordinate" => new VRML::NodeType("TextureCoordinate", [
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		point => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureCoordinateNode"),

	"TextureCoordinateGenerator" => new VRML::NodeType("TextureCoordinateGenerator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mode => ["SFString","SPHERE","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		parameter => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureCoordinateNode"),

	"TextureProperties" => new VRML::NodeType("TextureProperties", [
		anisotropicDegree => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		borderColor=>["SFColorRGBA",[0,0,0,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		borderWidth => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		boundaryModeS => ["SFString", "REPEAT", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		boundaryModeT => ["SFString", "REPEAT", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		boundaryModeR => ["SFString", "REPEAT", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		magnificationFilter => ["SFString", "FASTEST", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		minificationFilter => ["SFString", "FASTEST", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureCompression => ["SFString", "FASTEST", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texturePriority => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		generateMipMaps => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

	], "X3DSFNode"),

	"TextureTransform" => new VRML::NodeType ("TextureTransform", [
		center => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec2f", [1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		translation => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureTransformNode"),


	###################################################################################

	#	Chapter 19:		Interpolation Component

	###################################################################################

	"ColorInterpolator" => new VRML::NodeType("ColorInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFColor", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFColor", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),

	"CoordinateInterpolator" => new VRML::NodeType("CoordinateInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_GPU_Routes_out => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_CPU_Routes_out => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		# GPU running only - run the interpolator on the GPU, use these...
		_keyVBO =>["SFInt32", 0, "initializeOnly", 0,0],#ff
		_keyValueVBO =>["SFInt32", 0, "initializeOnly", 0,0],#ff
	],"X3DInterpolatorNode"),

	"CoordinateInterpolator2D" => new VRML::NodeType("CoordinateInterpolator2D", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["MFVec2f", [[0,0]], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),

	"EaseInEaseOut" => new VRML::NodeType("EaseInEaseOut", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		easeInEaseOut => ["MFVec2f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		modifiedFraction_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),



	"NormalInterpolator" => new VRML::NodeType("NormalInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),

	"OrientationInterpolator" => new VRML::NodeType("OrientationInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFRotation", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),

	"PositionInterpolator" => new VRML::NodeType("PositionInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),

	"PositionInterpolator2D" => new VRML::NodeType("PositionInterpolator2D", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),

	"ScalarInterpolator" => new VRML::NodeType("ScalarInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DInterpolatorNode"),

	"SplinePositionInterpolator" => new VRML::NodeType("SplinePositionInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		closed => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		keyVelocity => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFVec3f", [0,0,0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_T0 => ["MFVec3f", [], "initializeOnly", 0,0],#ff
		_T1 => ["MFVec3f", [], "initializeOnly", 0,0],#ff
	],"X3DInterpolatorNode"),

	"SplinePositionInterpolator2D" => new VRML::NodeType("SplinePositionInterpolator2D", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		closed => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		keyVelocity => ["MFVec2f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFVec2f", [0,0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_T0 => ["MFVec2f", [], "initializeOnly", 0,0],#ff
		_T1 => ["MFVec2f", [], "initializeOnly", 0,0],#ff
	],"X3DInterpolatorNode"),

	"SplineScalarInterpolator" => new VRML::NodeType("SplineScalarInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		closed => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyVelocity => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_T0 => ["MFFloat", [], "initializeOnly", 0,0],#ff
		_T1 => ["MFFloat", [], "initializeOnly", 0,0],#ff
	],"X3DInterpolatorNode"),

	"SquadOrientationInterpolator" => new VRML::NodeType("SquadOrientationInterpolator", [
		set_fraction => ["SFFloat", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFRotation", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalizeVelocity => ["SFBool", "FALSE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		closed => ["SFBool", "FALSE", "inputOutput", 0,0],#ff #H: the specs made a mistake it should be 'closed' not 'normalizeVelocity' -dug9
		value_changed => ["SFRotation", [0,0,1,0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_normkey => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_normkeyValue => ["MFRotation", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
  	],"X3DInterpolatorNode"),

	###################################################################################

	#		Cubemap Texturing Component

	###################################################################################


	"ComposedCubeMapTexture" => new VRML::NodeType("ComposedCubeMapTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		back =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bottom =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		front =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		left =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		top =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		right =>["SFNode","NULL","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DEnvironmentTextureNode"),

	"GeneratedCubeMapTexture" => new VRML::NodeType("GeneratedCubeMapTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__subTextures => ["MFNode",[],"initializeOnly",0,0],#ff
		__regenSubTextures => ["SFBool","FALSE","initializeOnly",0,0],#ff
		update => ["SFString","NONE","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["SFInt32",128,"initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
	],"X3DEnvironmentTextureNode"),

	#same order of fields up to __regenSubtextures as GeneratedCubeMapTexture
	"ImageCubeMapTexture" => new VRML::NodeType("ImageCubeMapTexture", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__subTextures => ["MFNode",[],"initializeOnly",0,0],#ff
		__regenSubTextures => ["SFBool","FALSE","initializeOnly",0,0],#ff
		url => ["MFString",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DEnvironmentTextureNode"),




	###################################################################################

	#	20	Pointing Device Component

	###################################################################################

	"TouchSensor" => new VRML::NodeType("TouchSensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hitNormal_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hitPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hitTexCoord_changed => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_oldhitNormal => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff 	# send event only if changed
		_oldhitPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff 	# send event only if changed
		_oldhitTexCoord => ["SFVec2f", [0, 0], "outputOnly", 0,0],#ff 	# send event only if changed
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		touchTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DPointingDeviceSensorNode"),

	"PlaneSensor" => new VRML::NodeType("PlaneSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		axisRotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxPosition => ["SFVec2f", [-1, -1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		minPosition => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		offset => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		translation_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sensorLocalOutput => ["SFBool", "FALSE", "initializeOnly", 0,"UNCA_NONE"],#ff
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		_oldtranslation => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		# where we are at a press...
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DPointingDeviceSensorNode"),

#
# Experimental node: LineSensor
#
	"LineSensor" => new VRML::NodeType("LineSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		direction => ["SFVec3f", [1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxPosition => ["SFFloat", -1, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		minPosition => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		offset => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		translation_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		_oldtranslation => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		# where we are at a press...
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DPointingDeviceSensorNode"),
	
#
# Experimental node: PointSensor
#
	
	"PointSensor" => new VRML::NodeType("PointSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxPosition => ["SFVec3f", [-1, -1, -1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		minPosition => ["SFVec3f",[0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		offset => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		translation_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		_oldtranslation => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		# where we are at a press...
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DPointingDeviceSensorNode"),

	

	"SphereSensor" => new VRML::NodeType("SphereSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		offset => ["SFRotation", [0, 1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		_oldrotation => ["SFRotation", [0, 0, 1, 0], "outputOnly", 0,0],#ff
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		# where we are at a press...
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		_origNormalizedPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		_radius => ["SFFloat", 0, "initializeOnly", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DPointingDeviceSensorNode"),

	"CylinderSensor" => new VRML::NodeType("CylinderSensor", [
		autoOffset => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		axisRotation => ["SFRotation", [0, 1, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		diskAngle => ["SFFloat", 0.262, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxAngle => ["SFFloat", -1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		minAngle => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		offset => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		rotation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		trackPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sensorLocalOutput => ["SFBool", "FALSE", "initializeOnly", 0,"UNCA_NONE"],#ff
		_oldtrackPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff
		_oldrotation => ["SFRotation", [0, 0, 1, 0], "outputOnly", 0,0],#ff
		# where we are at a press...
		_origPoint => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		_radius => ["SFFloat", 0, "initializeOnly", 0,0],#ff
		_usingDisk => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DPointingDeviceSensorNode"),


	###################################################################################

	#	21	Key Device Component

	###################################################################################

	# KeySensor
	"KeySensor" => new VRML::NodeType("KeySensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		actionKeyPress =>["SFInt32",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		actionKeyRelease =>["SFInt32",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		altKey =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlKey =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyPress =>["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyRelease =>["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		shiftKey =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DKeyDeviceSensorNode"),

	# StringSensor
	"StringSensor" => new VRML::NodeType("StringSensor", [
		deletionAllowed => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enteredText => ["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		finalText => ["SFString","","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive =>["SFBool", "TRUE","outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		singleton => ["SFBool", "TRUE", "inputOutput", 0,"UNCA_NONE"],#ff //if true, then shut off all other stringsensors when this enabled
		_initialized =>["SFBool", "FALSE","initializeOnly", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DKeyDeviceSensorNode"),


	###################################################################################

	#	22	Environmental Sensor Component

	###################################################################################


	"ProximitySensor" => new VRML::NodeType("ProximitySensor", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		position_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orientation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		centerOfRotation_changed =>["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# These fields are used for the info.
		__hit => ["SFInt32", 0, "inputOutput", 0,0],#ff
		__t1 => ["SFVec3f", [10000000, 0, 0], "inputOutput", 0,0],#ff
		__t2 => ["SFRotation", [0, 1, 0, 0], "inputOutput", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DEnvironmentalSensorNode"),

	"TransformSensor" => new VRML::NodeType("TransformSensor", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		position_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orientation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		targetObject => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# These fields are used for the info.
		__hit => ["SFInt32", 0, "inputOutput", 0,0],#ff
		__t1 => ["SFVec3f", [10000000, 0, 0], "inputOutput", 0,0],#ff
		__t2 => ["SFRotation", [0, 1, 0, 0], "inputOutput", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DEnvironmentalSensorNode"),


	"VisibilitySensor" => new VRML::NodeType("VisibilitySensor", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		 __visible =>["SFInt32",0,"initializeOnly", 0,0],#ff # for Occlusion tests.
		 __occludeCheckCount =>["SFInt32",-1,"initializeOnly", 0,0],#ff # for Occlusion tests.
		__points  =>["MFVec3f",[],"initializeOnly", 0,0],#ff	# for Occlude Box.
		__Samples =>["SFInt32",0,"initializeOnly", 0,0],#ff		# Occlude samples from last pass
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DEnvironmentalSensorNode"),



	###################################################################################

	#	23	Navigation Component

	###################################################################################

	"LOD" => new VRML::NodeType("LOD", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		level => ["MFNode", [], "inputOutput", "(SPEC_VRML)","UNCA_NONE"],#ff 		# for VRML spec
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff		# for X3D spec
		center => ["SFVec3f", [0, 0, 0],  "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		range => ["MFFloat", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		level_changed => ["SFInt32", 0, "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		forceTransitions => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__isX3D => ["SFBool", "(inputFileVersion[0]==3)" , "initializeOnly", 0,0],#ff # "TRUE" for X3D V3.x files
		_selected =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DGroupingNode"),

	"Billboard" => new VRML::NodeType("Billboard", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		axisOfRotation => ["SFVec3f", [0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_rotationAngle =>["SFDouble", 0, "initializeOnly", 0,0],#ff
		#JAS _sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	],"X3DGroupingNode"),

	"Collision" => new VRML::NodeType("Collision", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		collide => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		proxy => ["SFNode", "NULL", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		collideTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		#JAS _sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
		# return info for collisions
		# bit 0 : collision or not
		# bit 1: changed from previous of not
		__hit => ["SFInt32", 0, "inputOutput", 0,0],#ff
	],"X3DEnvironmentalSensorNode"),


	"Viewpoint" => new VRML::NodeType("Viewpoint", [
		#generic Viewpoint fields
		_layerId => ["SFInt32",0,"initializeOnly",0,0],#ff
		_donethispass => ["SFInt32",0,"initializeOnly",0,0],#ff
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		jump => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fieldOfView => ["SFFloat", 0.785398, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		position => ["SFVec3f",[0, 0, 10], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		centerOfRotation =>["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		retainUserOffsets => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# augmented reality extensions:
		fovMode => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		aspectRatio => ["SFFloat", 0.785398, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# user offsets:
		_initializedOnce => ["SFBool", "FALSE", "inputOnly", 0,0],#ff
		_orientation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,0],#ff
		_position => ["SFVec3f",[0, 0, 0], "initializeOnly", 0,0],#ff
	],"X3DBindableNode"),

	"OrthoViewpoint" => new VRML::NodeType("OrthoViewpoint", [
		#generic Viewpoint fields
		_layerId => ["SFInt32",0,"initializeOnly",0,0],#ff
		_donethispass => ["SFInt32",0,"initializeOnly",0,0],#ff
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		jump => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fieldOfView => ["MFFloat", [-1.0, -1.0, 1.0, 1.0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		position => ["SFVec3f",[0, 0, 10], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		centerOfRotation =>["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		retainUserOffsets => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# user offsets:
		_initializedOnce => ["SFBool", "FALSE", "inputOnly", 0,0],#ff
		_orientation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,0],#ff
		_position => ["SFVec3f",[0, 0, 0], "initializeOnly", 0,0],#ff
	],"X3DBindableNode"),



	"NavigationInfo" => new VRML::NodeType("NavigationInfo", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		avatarSize => ["MFFloat", [0.25, 1.6, 0.75], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		headlight => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		speed => ["SFFloat", 1.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		type => ["MFString", ["EXAMINE", "ANY"], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		visibilityLimit => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_layerId => ["SFInt32",0,"initializeOnly",0,0],#ff
		transitionType => ["MFString", [],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transitionTime => ["SFTime", 1.0, "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transitionComplete => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DBindableNode"),

	"ViewpointGroup" => new VRML::NodeType("ViewpointGroup", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		displayed => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		retainUserOffsets => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		__proxNode=> ["SFNode", "NULL", "inputOutput", "0","UNCA_NONE"],#ff
	],"X3DGroupingNode"),



	###################################################################################

	#	24	Environmental Effects Component

	###################################################################################

	"Background" => new VRML::NodeType("Background", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		groundAngle => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		groundColor => ["MFColor", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skyAngle => ["MFFloat", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		skyColor => ["MFColor", [[0, 0, 0]], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_layerId => ["SFInt32",0,"initializeOnly",0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__points =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		__colours =>["MFColor",[],"initializeOnly", 0,0],#ff
		__quadcount => ["SFInt32",0,"initializeOnly", 0,0],#ff
		#__combined => ["SFNode","NULL","initializeOnly",0,0],#ff
		transparency => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		frontUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		backUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		topUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bottomUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		leftUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rightUrl => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureright => ["SFInt32", 0, "inputOutput", 0,0],#ff
		__frontTexture=>["SFNode","NULL","inputOutput", 0,0],#ff
		__backTexture=>["SFNode","NULL","inputOutput", 0,0],#ff
		__topTexture=>["SFNode","NULL","inputOutput", 0,0],#ff
		__bottomTexture=>["SFNode","NULL","inputOutput", 0,0],#ff
		__leftTexture=>["SFNode","NULL","inputOutput", 0,0],#ff
		__rightTexture=>["SFNode","NULL","inputOutput", 0,0],#ff

		__VBO=>["SFInt32",0,"initializeOnly",0,0],#ff  # Vertex Buffer Object, if required.
	],"X3DBackgroundNode"),



	"Fog" => new VRML::NodeType("Fog", [
		#Fog interface - keep same order, offsets as LocalFog
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogType => ["SFString", "LINEAR", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		visibilityRange => ["SFFloat", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		__fogScale => ["SFFloat", 1.0, "inputOutput", 0,0],#ff
		__fogType => ["SFInt32",1,"initializeOnly",0,0],#ff
		#Bindable interface
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_layerId => ["SFInt32",0,"initializeOnly",0,0],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	  ],"X3DBindableNode"),

	"FogCoordinate" => new VRML::NodeType("FogCoordinate", [
		depth => ["MFFloat", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DGeometricPropertyNode"),

	"LocalFog" => new VRML::NodeType("Fog", [
		#Fog interface - keep same order, offsets as Fog
		color => ["SFColor", [1, 1, 1], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogType => ["SFString", "LINEAR", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		visibilityRange => ["SFFloat", 0, "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		__fogScale => ["SFFloat", 1.0, "inputOutput", 0,0],#ff
		__fogType => ["SFInt32",1,"initializeOnly",0,0],#ff
		#other
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),

	"TextureBackground" => new VRML::NodeType("TextureBackground", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		groundAngle => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		groundColor => ["MFColor", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skyAngle => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		skyColor => ["MFColor", [[0,0,0]], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_layerId => ["SFInt32",0,"initializeOnly",0,0],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__points =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		__colours =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		__quadcount => ["SFInt32",0,"initializeOnly", 0,0],#ff
		#__combined => ["SFNode","NULL","initializeOnly",0,0],#ff
		__VBO=>["SFInt32",0,"initializeOnly",0,0],#ff  # Vertex Buffer Object, if required.

		frontTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		backTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		topTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bottomTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		leftTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rightTexture=>["SFNode","NULL","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transparency=> ["MFFloat",[0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DBackgroundNode"),

	# see augmented reality for 2 more background nodes
	
	
	###################################################################################

	#	25	Geospatial Component

	###################################################################################


	"GeoCoordinate" => new VRML::NodeType("GeoCoordinate", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		point => ["MFVec3d",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff #v3.2 GD degrees, v3.3 GD angle units # see note top of file
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__movedCoords => ["MFVec3f", [], "inputOutput", 0,0],#ff
	],"X3DCoordinateNode"),

	"GeoElevationGrid" => new VRML::NodeType("GeoElevationGrid", [
		set_height => ["MFDouble", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		yScale => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		creaseAngle => ["SFDouble", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		geoGridOrigin => ["SFVec3d",[0,0,0],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		height => ["MFDouble", [0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		xDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		xSpacing => ["SFDouble", 1.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff
		zDimension => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		zSpacing => ["SFDouble", 1.0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff

		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__autoOffset => ["SFVec3d",[0,0,0],"initializeOnly", 0,0],#ff
		__localOrient => ["SFVec4d",[0,0,1,0],"initializeOnly", 0,0],#ff
		__planets => ["MFInt32",[],"initializeOnly", 0,0],#ff
	],"X3DGeometryNode"),

	"GeoLOD" => new VRML::NodeType("GeoLOD", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# the following screws up routing in the old VRML parser, because children can
		# be an "EXPOSED_FIELD" AND an "EVENT_OUT", so by changing this to an ""inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"
		# we can have only one field, the EXPOSED_FIELD_children
		#children => ["MFNode",[],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		children => ["MFNode", [], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		level_changed =>["SFInt32",0,"outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		center => ["SFVec3d",[0,0,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff # see note top of file
		child1Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		child2Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		child3Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		child4Url =>["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		range => ["SFFloat",10.0,"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		rootUrl => ["MFString",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rootNode => ["MFNode",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff

		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__inRange =>["SFBool", "FALSE", "inputOutput", 0,0],#ff
		__child1Node => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__child2Node => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__child3Node => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__child4Node => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__rootUrl => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__childloadstatus => ["SFInt32",0,"inputOutput", 0,0],#ff
		__rooturlloadstatus => ["SFInt32",0,"inputOutput", 0,0],#ff

		# ProximitySensor copies.
		#__t1 => ["SFVec3d", [10000000, 0, 0], "inputOutput", 0,0],#ff
		__level => ["SFInt32",-1,"inputOutput", 0,0],#ff # only for debugging purposes
	],"X3DGroupingNode"),


	"GeoMetadata" => new VRML::NodeType("GeoMetadata", [
		data => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		summary => ["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),

	"GeoPositionInterpolator" => new VRML::NodeType("GeoPositionInterpolator", [
		set_fraction => ["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFVec3d",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geovalue_changed => ["SFVec3d",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__movedValue => ["MFVec3f", [], "inputOutput", 0,0],#ff
		__oldKeyPtr => ["MFFloat", "NULL", "outputOnly", 0,0],#ff
		__oldKeyValuePtr => ["MFVec3d", "NULL", "outputOnly", 0,0],#ff
	],"X3DInterpolatorNode"),


	"GeoProximitySensor" => new VRML::NodeType("ProximitySensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D32)","UNCA_GEO"],#ff
		center => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_GEO"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_LENGTH"],#ff
		centerOfRotation_changed =>["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		enterTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		exitTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoCoord_changed => ["SFVec3d",[0,0,0],"outputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		orientation_changed => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		position_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32)","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff


		# These fields are used for the info.
		__hit => ["SFInt32", 0, "inputOutput", 0,0],#ff
		__t1 => ["SFVec3f", [10000000, 0, 0], "inputOutput", 0,0],#ff
		__t2 => ["SFRotation", [0, 1, 0, 0], "inputOutput", 0,0],#ff
		__t3 => ["SFVec3d", [10000000, 0, 0], "inputOutput", 0,0],#ff

		# "compiled" versions of strings above
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__localOrient => ["SFVec4d", [0, 0, 1, 0], "inputOutput", 0,0],#ff
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
		__oldGeoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__oldSize => ["SFVec3f", [0, 0, 0], "inputOutput", 0,0],#ff
	],"X3DEnvironmentalSensorNode"),

	"GeoTouchSensor" => new VRML::NodeType("GeoTouchSensor", [
		description => ["SFString", "", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hitNormal_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hitPoint_changed => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hitTexCoord_changed => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hitGeoCoord_changed => ["SFVec3d", [0, 0, 0] ,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isOver => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		touchTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		_oldhitNormal => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff 	# send event only if changed
		_oldhitPoint => ["SFVec3f", [0, 0, 0], "outputOnly", 0,0],#ff 	# send event only if changed
		_oldhitTexCoord => ["SFVec2f", [0, 0], "outputOnly", 0,0],#ff 	# send event only if changed
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DPointingDeviceSensorNode"),


	"GeoTransform" => new VRML::NodeType ("GeoTransform", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", 0,0],#ff
		center => ["SFVec3f", [0, 0, 0], "inputOutput", 0,"UNCA_LENGTH"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_GEO"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D33)","UNCA_LENGTH"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D33)","UNCA_BLENGTH"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32)","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff

		# fields for reducing redundant calls
		__do_center => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_trans => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_scaleO => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_scale => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_anything => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff

		# "compiled" versions of strings above
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__localOrient => ["SFVec4d", [0, 0, 1, 0], "inputOutput", 0,0],#ff
		__oldGeoCenter => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__oldChildren => ["MFNode", [], "inputOutput", 0,0],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	],"X3DGroupingNode"),

	"GeoViewpoint" => new VRML::NodeType("GeoViewpoint", [
		# generic Viewpoint fields - except watch it, the position is double (vs viewpoint and orthoviewpoint - single)
		_layerId => ["SFInt32",0,"initializeOnly",0,0],#ff
		_donethispass => ["SFInt32",0,"initializeOnly",0,0],#ff
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime", -1, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		jump => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fieldOfView => ["SFFloat", 0.785398, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff # see note top of file, AND spec changed to in/out in 3.3
		position => ["SFVec3d",[0, 0, 100000], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff # ditto
		centerOfRotation => ["SFVec3d",[0, 0, 0], "inputOutput", "( SPEC_X3D33)","UNCA_NONE"],#ff
		# the following sets were in v3.2 but v3.3 changed position,orientation to [inout] and (I think) that's backward compat in freewrl
		# set_orientation => ["SFRotation", ["IO_FLOAT", "IO_FLOAT", "IO_FLOAT", "IO_FLOAT"], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		# set_position => ["SFVec3d", ["IO_FLOAT", "IO_FLOAT", "IO_FLOAT"], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		# GeoViewpoint fields
		headlight => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		navType => ["MFString", ["EXAMINE","ANY"],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		speedFactor => ["SFFloat",1.0,"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		retainUserOffsets => ["SFBool", "FALSE", "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		# user offsets:
		_initializedOnce => ["SFBool", "FALSE", "inputOnly", 0,0],#ff
		_orientation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,0],#ff
		_position => ["SFVec3d",[0, 0, 0], "initializeOnly", 0,0],#ff
		relativeHeight => ["SFBool", "FALSE", "initializeOnly", 0,0],#ff
		_resetRelativeHeight => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		_prepped_planet => ["SFInt32",0,"initializeOnly",0,0],#ff
		# "compiled" versions of strings above
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__movedPosition => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__movedOrientation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,0],#ff
		__movedOrientationB => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,0],#ff
		__movedgd => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff

		__oldSFString => ["SFString", "", "inputOutput", 0,0],#ff #the description field
		__oldFieldOfView => ["SFFloat", 0.785398, "inputOutput", 0,0],#ff
		__oldHeadlight => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
		__oldJump => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
		__oldMFString => ["MFString", [],"inputOutput", 0,0],#ff # the navType

	],"X3DBindableNode"),

	# deprecated in v3.3
	"GeoOrigin" => new VRML::NodeType("GeoOrigin", [
		geoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_GEO"],#ff #v3.2 GD degrees, v3.3 GD angle units
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		rotateYUp => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff

		# these are now static in CFuncs/GeoVRML.c
		# "compiled" versions of strings above
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__movedgd => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__oldgeoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__oldMFString => ["MFString", [],"inputOutput", 0,0],#ff # the navType
		__rotyup => ["SFVec4d", [0, 1, 0, 0], "inputOutput", 0,0],#ff

	],"X3DChildNode"),
	
	"GeoConvert" => new VRML::NodeType("GeoConvert", [
		set_geoCoords => ["SFVec3d", [0, 0, 0], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_GEO"],#ff #v3.2 GD degrees, v3.3 GD angle units
		set_gcCoords => ["SFVec3d", [0, 0, 0], "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_GEO"],#ff #v3.2 GD degrees, v3.3 GD angle units
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		gcCoords_changed => ["SFVec3d", [0, 0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_GEO"],#ff #v3.2 GD degrees, v3.3 GD angle units
		geoCoords_changed => ["SFVec3d", [0, 0, 0], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_GEO"],#ff #v3.2 GD degrees, v3.3 GD angle units
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__oldgeoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__oldgcCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
	],"X3DInterpolatorNode"),
	

	"GeoLocation" => new VRML::NodeType("GeoLocation", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_GEO"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geoOrigin => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		geoSystem => ["MFString",["GD","WE"],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		relativeHeight => ["SFBool", "FALSE", "initializeOnly", 0,0],#ff
		_gridHeight => ["SFDouble", "0.0", "inputOnly", 0,0],#ff

		# "compiled" versions of strings above
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		__position => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__movedCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__movedgd => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__localOrient => ["SFVec4d", [0, 0, 1, 0], "inputOutput", 0,0],#ff
		__offsetOrient => ["SFVec4d", [0, 0, 1, 0], "inputOutput", 0,0],#ff
		__oldgeoCoords => ["SFVec3d", [0, 0, 0], "inputOutput", 0,0],#ff
		__oldChildren => ["MFNode", [], "inputOutput", 0,0],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	],"X3DGroupingNode"),


	"GeoPlanet" => new VRML::NodeType("GeoPlanet", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		planetId => ["SFInt32", 0,"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 )","UNCA_NONE"],#ff
		# "compiled" versions of strings above
		__oldChildren => ["MFNode", [], "inputOutput", 0,0],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	],"X3DGroupingNode"),



	###################################################################################

	#	26	H-Anim Component

	###################################################################################

	"HAnimDisplacer" => new VRML::NodeType("HAnimDisplacer", [
		coordIndex => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		displacements => ["MFVec3f", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DGeometricPropertyNode"),

	"HAnimHumanoid" => new VRML::NodeType("HAnimHumanoid", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		info => ["MFString", [],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff # see note top of file
		joints => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation",[0,0,1,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec3f",[1,1,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		segments => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sites => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skeleton => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skin => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skinCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skinNormal => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		version => ["SFString","","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		viewpoints => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_JT => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_PVI => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_PVW => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff	
		_NV => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_origCoords => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_origNorms => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DChildNode"),

	"HAnimJoint" => new VRML::NodeType("HAnimJoint", [

		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		displacers => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		limitOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		llimit => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skinCoordIndex => ["MFInt32",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		skinCoordWeight => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stiffness => ["MFFloat",[0,0,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ulimit => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# fields for reducing redundant calls
		__do_center => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_trans => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_scaleO => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_scale => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_anything => ["SFInt32", 0, "initializeOnly", 0,0],#ff
	],"X3DChildNode"),

	"HAnimSegment" => new VRML::NodeType("HAnimSegment", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		centerOfMass => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		displacers => ["MFNode",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mass => ["SFFloat", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		momentsOfInertia =>["MFFloat", [0, 0, 0, 0, 0, 0, 0, 0, 0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_MOMENT"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_origCoords => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DChildNode"),



	"HAnimSite" => new VRML::NodeType("HAnimSite", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# fields for reducing redundant calls
		__do_center => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_trans => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_scaleO => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_scale => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__do_anything => ["SFInt32", 0, "initializeOnly", 0,0],#ff
	],"X3DGroupingNode"),


	###################################################################################

	#	27	NURBS Component

	###################################################################################

	"Contour2D" => new VRML::NodeType("Contour2D", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DSFNode"),


	"ContourPolyline2D" => new VRML::NodeType("ContourPolyline2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint => ["MFVec2d", [], "inputOutput","( SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff  #v3.1 changed to this...
		point => ["MFVec2f", [], "inputOutput","(SPEC_X3D30 )","UNCA_NONE"],#ff #...from this, because point not in specs
	],"X3DNurbsControlCurveNode"),

	"CoordinateDouble" => new VRML::NodeType("CoordinateDouble", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		point => ["MFVec3d", [], "inputOutput","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
	],"X3DCoordinateNode"),

	"NurbsCurve" => new VRML::NodeType("NurbsCurve", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		knot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tessellation => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		closed => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,0],#ff
		__points  =>["MFVec3f",[],"initializeOnly", 0,0],#ff
		__numPoints =>["SFInt32",0,"initializeOnly", 0,0],#ff
	],"X3DParametricGeometryNode"),

	"NurbsCurve2D" => new VRML::NodeType("NurbsCurve2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["MFVec2d",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		knot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tessellation => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		closed => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,0],#ff
	],"X3DNurbsControlCurveNode"),


	"NurbsOrientationInterpolator" => new VRML::NodeType("NurbsOrientationInterpolator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight  => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		knot => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_fraction => ["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFRotation",[0,0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_knot => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_xyzw => ["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_OK => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_knotrange => ["SFVec2f", [0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),

	"NurbsPatchSurface" => new VRML::NodeType("NurbsPatchSurface", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,0],#ff
	],"X3DNurbsSurfaceGeometryNode"),

	"NurbsPositionInterpolator" => new VRML::NodeType("NurbsPositionInterpolator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		knot => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_fraction => ["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_knot => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_xyzw => ["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_OK => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_knotrange => ["SFVec2f", [0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),

	"NurbsSet" => new VRML::NodeType("NurbsSet", [
		addGeometry => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeGeometry => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geometry => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tessellationScale => ["SFFloat",1.0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
	],"X3DChildNode"),

	"NurbsSurfaceInterpolator" => new VRML::NodeType("NurbsSurfaceInterpolator", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_fraction => ["SFVec2f",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		position_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal_changed => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_uKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_vKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_controlPoint =>["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_OK => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),

	"NurbsSweptSurface" => new VRML::NodeType("NurbsSweptSurface", [
		crossSectionCurve =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		trajectoryCurve => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		method => ["SFString", "FULL", "inputOnly", 0,0],#ff #TRANSLATE / FULL
		_patch => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
		_method => ["SFInt32",2,"initializeOnly",0,0],#ff #1. Suv = Tv + Cu and delegate to patch 2. insert xsection at each profile tess point, and skin
	],"X3DParametricGeometryNode"),

	"NurbsSwungSurface" => new VRML::NodeType("NurbsSwungSurface", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		profileCurve =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		trajectoryCurve => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_patch => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
	],"X3DParametricGeometryNode"),

	"NurbsTextureCoordinate" => new VRML::NodeType("NurbsTextureCoordinate", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["MFVec2f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_uKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_vKnot => ["MFFloat",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_controlPoint =>["MFVec4f",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DSFNode"),

	#TrimmedSurface == PatchSurface + trimmingContour - keep them in the same order so Trimmed can be downcast to Patch
	"NurbsTrimmedSurface" => new VRML::NodeType("NurbsTrimmedSurface", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		controlPoint =>["SFNode","NULL","inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weight => ["MFDouble",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		uClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vKnot => ["MFDouble",[],"initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vOrder => ["SFInt32",3,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vDimension => ["SFInt32",0,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vTessellation => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		vClosed => ["SFBool", "FALSE","initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		addTrimmingContour => ["MFNode",[],"inputOnly","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeTrimmingContour => ["MFNode",[],"inputOnly","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		trimmingContour =>["MFNode",[], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tscale => ["SFFloat",1.0,"initializeOnly", 0,0],#ff
	], "X3DNurbsSurfaceGeometryNode"),


	###################################################################################

	# Chapter 28: Distributed Interactive Simulation Component

	###################################################################################


	"DISEntityManager" => new VRML::NodeType("DISEntityManager", [
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mapping => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		addedEntities => ["MFNode", [], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removedEntities => ["MFNode", [], "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),

	"DISEntityTypeMapping" => new VRML::NodeType("DISEntityTypeMapping", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		kind => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		domain => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		country => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		category => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		subcategory => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		specific => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		extra => ["SFInt32", 0, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

	],"X3DInfoNode"),

	"EspduTransform" => new VRML::NodeType("EspduTransform", [
		# network sensor
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		timestamp => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		readInterval => ["SFTime", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		writeInterval => ["SFTime", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rtpHeaderExpected => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_registered => ["SFBool","FALSE","initializeOnly",0,"UNCA_NONE"],#ff
		_dsock => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],#ff
		_lasttime => ["SFTime",0,"initializeOnly",0,"UNCA_NONE"],#ff
		_pduchange_networksensor => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		# DIS Entity
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# Geo
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_GEO"],#ff
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff

		# Field Change Detection
		_oldState => ["SFNode","NULL","initializeOnly", 0,0],#ff

		# Info
		entityCategory => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		entityCountry => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		entityDomain => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		entityExtra => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		entityKind => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		entitySpecific => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		entitySubCategory => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		#_pduchange_es_info => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		# team / side / force
		forceID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		marking => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		#_pduchange_es_force => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		
		# DIS EntityState > deadReckoning
		deadReckoning => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		linearVelocity => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		linearAcceleration => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ACCEL"],#ff
		_p0 => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		_v0 => ["SFVec3f", [0,0,0], "initializeOnly", 0,0],#ff
		_a0 => ["SFVec3f", [0,0,0], "initializeOnly", 0,0],#ff
		_angularVelocity => ["SFRotation", [0,1,0,0], "initializeOnly", 0,0],#ff
		_r0 => ["SFRotation", [0,1,0,0], "initializeOnly", 0,0],#ff
		_change_count => ["SFInt32", 0, "inputOutput", 0,0],#ff
		_sent => ["SFInt32", 0, "inputOutput", 0,0],#ff
		_lastp0 => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		_lastr0 => ["SFRotation", [0,1,0,0], "initializeOnly", 0,0],#ff
		_lastp0time => ["SFTime",0,"initializeOnly",0,"UNCA_NONE"],#ff
		_lastframetime => ["SFTime",0,"initializeOnly",0,"UNCA_NONE"],#ff
		_smoothingDelta => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		_smoothingCount => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		#_pduchange_es_deadreckoning => ["SFInt32", 0, "initializeOnly", 0,0],#ff
	
		# DIS EntityState > articulationParameters
		set_articulationParameterValue0 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_articulationParameterValue1 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_articulationParameterValue2 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_articulationParameterValue3 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_articulationParameterValue4 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_articulationParameterValue5 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_articulationParameterValue6 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_articulationParameterValue7 => ["SFFloat", 0.0, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterCount => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterDesignatorArray => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterChangeIndicatorArr => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterIdPartAttachedToAr => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterTypeArray => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterArray => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue0_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue1_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue2_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue3_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue4_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue5_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue6_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		articulationParameterValue7_changed => ["SFFloat", 0.0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		#_pduchange_es_articulation => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_pduchange_es => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		# DIS collision
		collisionType => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		collideTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isCollided => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_pduchange_collision => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		
		# DIS shared fire/collision
		eventEntityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		eventApplicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		eventSiteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		eventNumber => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		
		# DIS fire (as in 'fire weapon')
		fired1 => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fired2 => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fireMissionIndex => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		firingRange => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		firedTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_pduchange_fire => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		

		# DIS detonation
		detonationLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		detonationRelativeLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		detonationResult => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		detonateTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isDetonated => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_pduchange_detonation => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		# DIS shared fire/detonation
		munitionEntityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		munitionApplicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		munitionSiteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		munitionStartPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		munitionEndPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		# > burst descriptor information
		munitionQuantity => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		firingRate => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fuse => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		warhead => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# DIS createEntityPdu / removeEntityPdu (not sure what / how this works)
		_pduchange_create => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_pduchange_remove => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		#start same order as Transform >>>
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		center => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation", [0,0,1,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec3f", [1,1,1], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scaleOrientation => ["SFRotation", [0,0,1,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		# fields for reducing redundant calls
		__do_center => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_scaleO => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_scale => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_anything => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
		# << end same order as Transform
		#_pduchange_es_transform => ["SFInt32", 0, "initializeOnly", 0,0],#ff

	], "X3DGroupingNode"),


	"ReceiverPdu" => new VRML::NodeType("ReceiverPdu", [
		# network sensor
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		timestamp => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		readInterval => ["SFTime", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		writeInterval => ["SFTime", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rtpHeaderExpected => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_registered => ["SFBool","FALSE","initializeOnly",0,"UNCA_NONE"],#ff
		_dsock => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],#ff
		_lasttime => ["SFTime",0,"initializeOnly",0,"UNCA_NONE"],#ff
		_pduchange_networksensor => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		# DIS Entity
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# Geo
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_GEO"],#ff
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		
		# Field Change Detection
		_oldState => ["SFNode","NULL","initializeOnly", 0,0],#ff
		
		# DIS Receiver
		radioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		whichGeometry => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		receiverState => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		receivedPower => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transmitterEntityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transmitterApplicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transmitterSiteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transmitterRadioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_pduchange_receiver => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		
	], "X3DChildNode"),

	"SignalPdu" => new VRML::NodeType("SignalPdu", [
		# network sensor
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		timestamp => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		readInterval => ["SFTime", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		writeInterval => ["SFTime", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rtpHeaderExpected => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_registered => ["SFBool","FALSE","initializeOnly",0,"UNCA_NONE"],#ff
		_dsock => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],#ff
		_lasttime => ["SFTime",0,"initializeOnly",0,"UNCA_NONE"],#ff
		_pduchange_networksensor => ["SFInt32", 0, "initializeOnly", 0,0],#ff

		# DIS Entity
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# Geo
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_GEO"],#ff
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff
		
		# Field Change Detection
		_oldState => ["SFNode","NULL","initializeOnly", 0,0],#ff
		
		# DIS SignalPdu
		radioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		whichGeometry => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		data => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		dataLength => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		encodingScheme => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sampleRate => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		samples => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tdlType => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_pduchange_signal => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		
	], "X3DChildNode"),

	"TransmitterPdu" => new VRML::NodeType("TransmitterPdu", [
		# network sensor
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		timestamp => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		address => ["SFString", "localhost", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		port => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayHost => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		multicastRelayPort => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		networkMode => ["SFString", "standAlone", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkReader => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isNetworkWriter => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isStandAlone => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		readInterval => ["SFTime", 0.1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		writeInterval => ["SFTime", 1.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rtpHeaderExpected => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isRtpHeaderHeard => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_registered => ["SFBool","FALSE","initializeOnly",0,"UNCA_NONE"],#ff
		_dsock => ["SFNode", "NULL", "initializeOnly", 0,"UNCA_NONE"],#ff
		_lasttime => ["SFTime",0,"initializeOnly",0,"UNCA_NONE"],#ff
		_pduchange_networksensor => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		

		# DIS Entity
		entityID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		applicationID => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		siteID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		# Geo
		geoSystem => ["MFString", ["GC","WE"], "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		geoCoords => ["SFVec3d", [0,0,0], "inputOutput", "(SPEC_X3D33)","UNCA_GEO"],#ff
		__geoSystem => ["SFNode","NULL","initializeOnly", 0,0],#ff

		# Field Change Detection
		_oldState => ["SFNode","NULL","initializeOnly", 0,0],#ff
		

		# DIS Transmitter
		radioID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		whichGeometry => ["SFInt32", 1, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radioEntityTypeCategory => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radioEntityTypeCountry => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radioEntityTypeDomain => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radioEntityTypeKind => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radioEntityTypeNomenclature => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		radioEntityTypeNomenclatureVersion => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		antennaLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		antennaPatternLength => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		antennaPatternType => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		relativeAntennaLocation => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff

		inputSource => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		transmitState => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		power => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		frequency => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transmitFrequencyBandwidth => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		lengthOfModulationParameters => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		modulationTypeDetail => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		modulationTypeMajor => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		modulationTypeSpreadSpectrum => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		modulationTypeSystem => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		cryptoSystem => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		cryptoKeyID => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_pduchange_transmitter => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		
	], "X3DChildNode"),





	###################################################################################

	#	29.	Scripting Component

	###################################################################################
	"Script" => new VRML::NodeType("Script", [
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		directOutput => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mustEvaluate => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__scriptObj => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DScriptNode"),

	###################################################################################

	#	32.	CAD Component

	###################################################################################

	"CADAssembly" => new VRML::NodeType("CADAssembly", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	], "X3DGroupingNode"),

	"CADFace" => new VRML::NodeType("CADFace", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		shape => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
	], "X3DProductStructureChildNode"),

	"CADLayer" => new VRML::NodeType("CADLayer", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		visible => ["MFBool", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
	], "X3DGroupingNode"),

	"CADPart" => new VRML::NodeType("CADPart", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		center => ["SFVec3f",[0,0,0],"inputOutput","(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scaleOrientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff

		# fields for reducing redundant calls
		__do_center => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_scaleO => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_scale => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_anything => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		_sortedChildren => ["MFNode", [], "inputOutput", 0,0],#ff
	] ,"X3DGroupingNode"),

	"IndexedQuadSet" => new VRML::NodeType("IndexedQuadSet", [
		set_index => ["MFInt32", undef, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		attrib  => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		index => ["MFInt32", [], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	], "X3DComposedGeometryNode"),

	"QuadSet" => new VRML::NodeType("QuadSet", [
		attrib  => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fogCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normal => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoord => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		ccw => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		normalPerVertex => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		solid => ["SFBool", "TRUE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_coordIndex => ["MFInt32", [], "initializeOnly", 0,0],#ff
	], "X3DComposedGeometryNode"),


	###################################################################################

	#	30.	EventUtilities Component

	###################################################################################

	"BooleanFilter" => new VRML::NodeType("BooleanFilter", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		inputFalse => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		inputNegate => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		inputTrue => ["SFBool", "TRUE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),


	"BooleanSequencer" => new VRML::NodeType("BooleanSequencer", [
		next =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		previous =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_fraction =>["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFBool", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_index => ["SFInt32", -1, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff web3d.org debate 0 vs -1
	],"X3DSequencerNode"),


	"BooleanToggle" => new VRML::NodeType("BooleanToggle", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		toggle => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DChildNode"),


	"BooleanTrigger" => new VRML::NodeType("BooleanTrigger", [
		set_triggerTime => ["SFTime",undef ,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		triggerTrue => ["SFBool", "FALSE", "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTriggerNode"),


	"IntegerSequencer" => new VRML::NodeType("IntegerSequencer", [
		next =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		previous =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_fraction =>["SFFloat",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		key => ["MFFloat", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		keyValue => ["MFInt32", [], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value_changed => ["SFInt32", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_index => ["SFInt32", -1, "initializeOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DSequencerNode"),

	"IntegerTrigger" => new VRML::NodeType("IntegerTrigger", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		integerKey => ["SFInt32", 0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		triggerValue => ["SFInt32", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTriggerNode"),

	"TimeTrigger" => new VRML::NodeType("TimeTrigger", [
		set_boolean =>["SFBool",undef,"inputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		triggerTime => ["SFTime", 0, "outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTriggerNode"),


	###################################################################################

	#	31.	ProgrammableShaders Component

	###################################################################################

	"ComposedShader" => new VRML::NodeType("ComposedShader", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		parts => ["MFNode",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		language => ["SFString", "", "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,0],#ff
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,0],#ff
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
	],"X3DShaderNode"),


	"FloatVertexAttribute" => new VRML::NodeType("FloatVertexAttribute", [
		value => ["MFFloat",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		numComponents => ["SFInt32", 4, "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # 1...4 valid values
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DVertexAttributeNode"),

	"Matrix3VertexAttribute" => new VRML::NodeType("Matrix3VertexAttribute", [
		value => ["MFMatrix3f",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DVertexAttributeNode"),

	"Matrix4VertexAttribute" => new VRML::NodeType("Matrix4VertexAttribute", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		value => ["MFMatrix4f",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		name => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DVertexAttributeNode"),

	"PackagedShader" => new VRML::NodeType("PackagedShader", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		language => ["SFString","","initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,0],#ff
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,0],#ff
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
	], "X3DProgrammableShaderObject"),

	"ProgramShader" => new VRML::NodeType("ProgramShader", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		programs => ["MFNode", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		language => ["SFString","","initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,0],#ff
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,0],#ff
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
	], "X3DProgrammableShaderObject"),

	"ShaderPart" => new VRML::NodeType("ShaderPart", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		type => ["SFString","VERTEX","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
	], "X3DUrlObject"),

	"ShaderProgram" => new VRML::NodeType("ShaderProgram", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		type => ["SFString","","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
	], "X3DUrlObject"),

	# castle EffectPart made from ShaderPart - fields in same order
	"EffectPart" => new VRML::NodeType("EffectPart", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		type => ["SFString","VERTEX","inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		__loadstatus =>["SFInt32",0,"initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__loadResource => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
	], "X3DUrlObject"),

	# castle Effect made from ComposedShader - fields in same order
	"Effect" => new VRML::NodeType("Effect", [
		activate =>["SFBool",undef,"inputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		parts => ["MFNode",[],"inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isSelected => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isValid => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		language => ["SFString", "", "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_initialized => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
		_shaderUserDefinedFields => ["SFNode", "NULL", "initializeOnly", 0,0],#ff
		_shaderUserNumber => ["SFInt32",-1,"initializeOnly",0,0],#ff
		_shaderLoadThread => ["FreeWRLThread", 0, "initializeOnly",0,0],#ff
		_retrievedURLData => ["SFBool", "FALSE" ,"initializeOnly", 0,0],#ff
	],"X3DShaderNode"),


	###################################################################################

	#	33.	Texturing3D Component

	###################################################################################
	"ImageTexture3D" => new VRML::NodeType("ImageTexture3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatS => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatT => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatR => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_needs_gradient => ["SFBool", "FALSE", "initializeOnly", 0,0],#ff
	],"X3DTextureNode"),

	"PixelTexture3D" => new VRML::NodeType("PixelTexture3D", [
		image => ["MFInt32", "0, 0, 0, 0", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatS => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatT => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatR => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_needs_gradient => ["SFBool", "FALSE", "initializeOnly", 0,0],#ff
	],"X3DTextureNode"),

	"TextureCoordinate3D" => new VRML::NodeType("TextureCoordinate3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		point => ["MFVec3f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureCoordinateNode"),

	"TextureCoordinate4D" => new VRML::NodeType("TextureCoordinate4D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		point => ["MFVec4f", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureCoordinateNode"),

	"TextureTransformMatrix3D" => new VRML::NodeType("TextureTransformMatrix3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		matrix => ["SFMatrix4f", [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureTransformNode"),

	"TextureTransform3D" => new VRML::NodeType ("TextureTransform3D", [
		center => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		scale => ["SFVec3f", [1, 1, 1], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DTextureTransformNode"),

	"ComposedTexture3D" => new VRML::NodeType("ComposedTexture3D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texture=>["MFNode",undef,"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		textureProperties => ["SFNode", 0, "initializeOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		repeatS => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatT => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		repeatR => ["SFBool", "FALSE", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__textureTableIndex => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		_parentResource =>["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
	],"X3DTexture3DNode"),


	###################################################################################

	#	35.	Layering Component

	###################################################################################

	"Viewport" => new VRML::NodeType("Viewport", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		clipBoundary => ["MFFloat",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
	], "X3DViewportNode"),

	"Layer" => new VRML::NodeType("Layer", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isPickable => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		viewport => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DLayerNode"),

	"LayerSet" => new VRML::NodeType("LayerSet", [
		activeLayer => ["SFInt32", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		layers => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["MFInt32",[0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DLayerSetNode"),

	###################################################################################

	#	36.	Layout Component

	###################################################################################
	"Layout" => new VRML::NodeType("Layout", [
		align => ["MFString", ["CENTER","CENTER"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		offset => ["MFFloat",[0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		offsetUnits => ["MFString", ["WORLD","WORLD"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		scaleMode => ["MFString", ["NONE","NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		size => ["MFFloat",[1,1],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sizeUnits => ["MFString", ["WORLD","WORLD"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_align => ["MFInt32",[0,0],"initializeOnly", 0,0],#ff
		_offsetUnits => ["MFInt32",[0,0], "initializeOnly", 0,0],#ff
		_scaleMode => ["MFInt32",[0,0], "initializeOnly", 0,0],#ff
		_sizeUnits => ["MFInt32",[0,0], "initializeOnly", 0,0],#ff
		_scale => ["MFFloat",[1,1], "initializeOnly", 0,0],#ff
	], "X3DLayoutNode"),

	"LayoutGroup" => new VRML::NodeType("LayoutGroup", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		layout => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		viewport => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DGroupingNode"),



	"LayoutLayer" => new VRML::NodeType("LayoutLayer", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isPickable => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		viewport => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		layout => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DGroupingNode"),

	"ScreenFontStyle" => new VRML::NodeType("ScreenFontStyle", [
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		family => ["MFString", ["SERIF"], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		horizontal => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		justify => ["MFString", ["BEGIN"], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		language => ["SFString", "", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		leftToRight => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pointSize => ["SFFloat", 12.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		spacing => ["SFFloat", 1.0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		style => ["SFString", "PLAIN", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		topToBottom => ["SFBool", "TRUE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DFontStyleNode"),


	"ScreenGroup" => new VRML::NodeType("ScreenGroup", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
	], "X3DGroupingNode"),

	###################################################################################

	#	37.	Rigid Body Physics Component

	###################################################################################

	"BallJoint" => new VRML::NodeType("BallJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_VRML | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_forceout => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
	], "X3DRigidJointNode"),

	"CollidableOffset" => new VRML::NodeType("CollidableOffset", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		collidable => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_geom => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_initialRotation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,0],#ff 
		_initialTranslation => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		_initialized => ["SFBool",0,"initializeOnly",0,0],#ff
		_csensor => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	], "X3DNBodyCollidableNode"),

	"CollidableShape" => new VRML::NodeType("CollidableShape", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		translation => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		__do_trans => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		__do_rotation => ["SFInt32", "FALSE", "initializeOnly", 0,0],#ff
		shape => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_geom => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_initialRotation => ["SFRotation", [0, 0, 1, 0], "initializeOnly", 0,0],#ff 
		_initialTranslation => ["SFVec3f", [0, 0, 0], "initializeOnly", 0,0],#ff
		_initialized => ["SFBool",0,"initializeOnly",0,0],#ff
		_csensor => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	], "X3DNBodyCollidableNode"),

	"CollisionCollection" => new VRML::NodeType("CollisionCollection", [
		appliedParameters => ["MFString", ["BOUNCE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bounce => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		collidables => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		frictionCoefficients => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		minBounceSpeed => ["SFFloat", 0.1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff # see note top of file
		slipFactors => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		softnessConstantForceMix => ["SFFloat", 0.0001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_FORCE"],#ff # see note top of file
		softnessErrorCorrection => ["SFFloat", 0.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
		surfaceSpeed => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		_class => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_csensor => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_appliedParametersMask => ["SFInt32", 0, "initializeOnly", 0,0],#ff
	],"X3DChildNode"),

	"CollisionSensor" => new VRML::NodeType("CollsionSensor", [
		collider => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intersections => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		contacts => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "TRUE", "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	], "X3DSensorNode"),

	"CollisionSpace" => new VRML::NodeType("CollisionSpace", [
		collidables => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		useGeometry => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1,-1,-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_space => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	], "X3DNBodyCollidableNode"),

	"Contact" => new VRML::NodeType("Contact", [
		appliedParameters => ["MFString", ["BOUNCE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bounce => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		contactNormal => ["SFVec3f", [0,1,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		depth => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		frictionCoefficients => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		frictionDirection => ["SFVec3f", [0,1,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geometry1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geometry2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		minBounceSpeed => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		position => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		slipCoefficients => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		softnessConstantForceMix => ["SFFloat", 0.0001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_FORCE"],#ff
		softnessErrorCorrection => ["SFFloat", 0.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceSpeed => ["SFVec2f", [0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		_appliedParameters => ["SFInt32", 0, "initializeOnly", 0,0],#ff
	], "X3DSFNode"),

	"DoubleAxisHingeJoint" => new VRML::NodeType("DoubleAxisHingeJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		axis1 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		axis2 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		desiredAngularVelocity1 => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLERATE"],#ff
		desiredAngularVelocity2 => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLERATE"],#ff
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxAngle1 => ["SFFloat", "PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		maxTorque1 => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_TORQUE"],#ff
		maxTorque2 => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_TORQUE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		minAngle1 => ["SFFloat", "-PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		stopBounce1 => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stopConstantForceMix1 => ["SFFloat", .001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_FORCE"],#ff
		stopErrorCorrection1 => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		suspensionErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		suspensionForce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_FORCE"],#ff
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		body1Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		body2Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		hinge1Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		hinge1AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLERATE"],#ff
		hinge2Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		hinge2AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLERATE"],#ff
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_forceout => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_axis1 => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_axis2 => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		_motor1 => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_motor2 => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		axis1Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff  #NOT IN SPECS, DO WE USE THIS?
	], "X3DRigidJointNode"),

	"MotorJoint" => new VRML::NodeType("MotorJoint", [
		axis1Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		axis1Torque => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_TORQUE"],#ff
		axis2Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		axis2Torque => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_TORQUE"],#ff
		axis3Angle => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		axis3Torque => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_TORQUE"],#ff
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabledAxes => ["SFInt32", 1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor1Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor2Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor3Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop1Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop1ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop2Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop2ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop3Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop3ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		motor1Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor1AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor2Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor2AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor3Angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		motor3AngleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		autoCalc => ["SFBool", "FALSE", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_forceout => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__old_motor1Axis => ["SFVec3f",[0,0,0],"outputOnly", 0,0],#ff
		__old_motor2Axis => ["SFVec3f",[0,0,0],"outputOnly", 0,0],#ff
		__old_motor3Axis => ["SFVec3f",[0,0,0],"outputOnly", 0,0],#ff
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__old_axis1Angle => ["SFFloat", 0, "inputOutput", 0,0],#ff
		__old_axis2Angle => ["SFFloat", 0, "inputOutput", 0,0],#ff
		__old_axis3Angle => ["SFFloat", 0, "inputOutput", 0,0],#ff
	], "X3DRigidJointNode"),

	"RigidBody" => new VRML::NodeType("RigidBody", [
		angularDampingFactor => ["SFFloat", 0.001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff  #or UNCA_ANGLRATE
		angularVelocity => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLERATE"],#ff
		autoDamp => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		autoDisable => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		centerOfMass => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		disableAngularSpeed => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLERATE"],#ff
		disableLinearSpeed => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		disableTime => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		finiteRotationAxis => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fixed => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		forces => ["MFVec3f",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_FORCE"],#ff
		geometry => ["MFNode",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		inertia => ["SFMatrix3f",[1,0,0,0,1,0,0,0,1],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MOMENT"],#ff
		linearDampingFactor => ["SFFloat", 0.001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		linearVelocity => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		mass => ["SFFloat", 1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		massDensityModel => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		position => ["SFVec3f", [0, 0, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		torques => ["MFVec3f",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_TORQUE"],#ff
		useFiniteRotation => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		useGlobalGravity => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_body => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		__old_angularVelocity => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_centerOfMass => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_finiteRotationAxis => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_linearVelocity => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_orientation => ["SFRotation", [0, 0, 1, 0], "inputOutput", 0,0],#ff
		__old_position => ["SFVec3f", [0, 0, 0], "inputOutput", 0,0],#ff
		_geomIdentityTransform => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	], "X3DSFNode"),

	"RigidBodyCollection" => new VRML::NodeType("RigidBodyCollection", [
		set_contacts =>["MFNode",[],"inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		autoDisable => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bodies => ["MFNode",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		constantForceMix => ["SFFloat", .0001, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		contactSurfaceThickness => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		disableAngularSpeed => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		disableLinearSpeed => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		disableTime => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		errorCorrection => ["SFFloat", 0.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		gravity => ["SFVec3f", [0, -9.8, 0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_FORCE"],#ff
		iterations => ["SFInt32",10,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		joints => ["MFNode",[],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxCorrectionSpeed => ["SFFloat", -1.8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		preferAccuracy => ["SFBool", "FALSE", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		collider => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_world => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		#_space => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_group => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
	], "X3DChildNode"),

	"SingleAxisHingeJoint" => new VRML::NodeType("SingleAxisHingeJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		axis => ["SFVec3f", [0,0,1], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxAngle => ["SFFloat", "PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		minAngle => ["SFFloat", "-PIF+", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		stopBounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stopErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		angle=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		angleRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_forceout => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_axis => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
	], "X3DRigidJointNode"),

	"SliderJoint" => new VRML::NodeType("SliderJoint", [
		axis => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxSeparation => ["SFFloat", 1.0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		minSeparation => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		sliderForce => ["SFFloat", 0, "inputOutput", "( SPEC_X3D33)","UNCA_FORCE"],#ff
		stopBounce => ["SFFloat",  0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stopErrorCorrection => ["SFFloat",  1, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		separation => ["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		separationRate=>["SFFloat",0.0,"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_forceout => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__old_axis => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
	], "X3DRigidJointNode"),
	
	"UniversalJoint" => new VRML::NodeType("UniversalJoint", [
		anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		axis1 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		axis2 => ["SFVec3f", [0,0,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2 => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		forceOutput => ["MFString", ["NONE"], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop1Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop1ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop2Bounce => ["SFFloat", 0, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stop2ErrorCorrection => ["SFFloat", .8, "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body1Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2AnchorPoint => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		body2Axis => ["SFVec3f",[0,0,0],"outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_joint => ["FreeWRLPTR", 0, "initializeOnly", 0,0],#ff
		_forceout => ["SFInt32", 0, "initializeOnly", 0,0],#ff
		__old_anchorPoint => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_axis1 => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_axis2 => ["SFVec3f", [0,0,0], "inputOutput", 0,0],#ff
		__old_body1 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
		__old_body2 => ["SFNode", "NULL", "inputOutput", 0,0],#ff
	], "X3DRigidJointNode"),


	###################################################################################

	#	38.	Picking Component

	###################################################################################
	
	# A PickableGroup node is an X3DGroupingNode that contains children that are marked
	# as being of a given classification of picking types, as well as the ability to enable or disable picking of the children.

# DJTRACK_PICKSENSORS
	"PickableGroup" => new VRML::NodeType("PickableGroup", [
		addChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		removeChildren => ["MFNode", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__sibAffectors => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		children => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickable => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff

		#FreeWRL__protoDef => ["SFInt32", "INT_ID_UNDEFINED", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)"], # tell renderer that this is a proto...
		#FreeWRL_PROTOInterfaceNodes =>["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DGroupingNode"),

	# The PointPickSensor node tests one or more points in space as lying inside the provided target geometry.
	# For each point that lies inside the geometry, the point coordinate is returned in the pickedGeometry field
	# with the corresponding geometry inside which the point lies.
	# Because points represent an infinitely small location in space, the "CLOSEST" and "ALL_SORTED" sort orders
	# are defined to mean "ANY" and "ALL" respectively.

	"PointPickSensor" => new VRML::NodeType("PointPickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_X3D33)","UNCA_NONE"],#ff
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
		pickedPoint => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		
		#DJTRACK
		_oldisActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_oldpickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_oldpickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_oldpickedPoint => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		set_intersectionType => ["SFString", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_sortOrder => ["SFString", undef, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DSensorNode"),

	"LinePickSensor" => new VRML::NodeType("LinePickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
		pickedPoint => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickedNormal => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickedTextureCoordinate => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DSensorNode"),
	
	#38.4.4 PrimitivePickSensor
	"PrimitivePickSensor" => new VRML::NodeType("PrimitivePickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DSensorNode"),
	
	
	#38.4.5 VolumePickSensor
	"VolumePickSensor" => new VRML::NodeType("VolumePickSensor", [
		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		objectType => ["MFString", ["ALL","NONE","TERRAIN"],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickingGeometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickTarget => ["MFNode", [], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		pickedGeometry => ["MFNode", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intersectionType => ["SFString","BOUNDS","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		sortOrder => ["SFString","CLOSEST","initializeOnly", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		matchCriterion => ["SFString","MATCH_ANY","inputOutput", "(SPEC_VRML | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		# These fields are used for the info.
		__oldEnabled => ["SFBool", "TRUE", "inputOutput", 0,0],#ff
	],"X3DSensorNode"),
	
	###################################################################################

	#	39.	Followers Component

	###################################################################################
	
	# value_changed is the first field-type-sepcific field so that offsetof(,value_changed) will be generic for all chasers, and for all dampers
	"ColorChaser" => new VRML::NodeType("ColorChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_steptime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		value_changed => ["SFColor", [0,0,0], "outputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFColor", [.8,.8,.8], "initializeOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialValue => ["SFColor", [.8,.8,.8], "initializeOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_destination => ["SFColor", [0,0,0], "inputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFColor", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_previousvalue => ["SFColor", [0,0,0], "initializeOnly", 0,0],#ff
		_destination => ["SFColor", [0,0,0], "initializeOnly", 0,0],#ff
	],"X3DChaserNode"),
	
	"ColorDamper" => new VRML::NodeType("ColorDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		tau  => ["SFTime", .3,"inputOutput", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		value_changed => ["SFColor", [0,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFColor", [.8,.8,.8], "initializeOnly", "( SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialValue => ["SFColor", [.8,.8,.8], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_destination => ["SFColor", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFColor", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_input => ["SFColor", [0,0,0], "initializeOnly", 0,0],#ff
		
	],"X3DDamperNode"),
	
	"CoordinateChaser" => new VRML::NodeType("CoordinateChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_steptime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		initialValue => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		set_destination => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_previousvalue => ["MFVec3f", [[0,0,0]], "initializeOnly", 0,0],#ff
		_destination => ["MFVec3f", [[0,0,0]], "initializeOnly", 0,0],#ff
	],"X3DChaserNode"),
	
	"CoordinateDamper" => new VRML::NodeType("CoordinateDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		value_changed => ["MFVec3f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		initialValue => ["MFVec3f", [[0,0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		set_destination => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["MFVec3f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_input => ["MFVec3f", [], "initializeOnly", 0,0],#ff
	],"X3DDamperNode"),

	"OrientationChaser" => new VRML::NodeType("OrientationChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_steptime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		value_changed => ["SFRotation", [0,1,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		initialValue => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		set_destination => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_previousvalue => ["SFRotation", [0,1, 0,0], "initializeOnly", 0,0],#ff
		_destination => ["SFRotation", [0,1,0,0], "initializeOnly", 0,0],#ff
	],"X3DChaserNode"),
	
	"OrientationDamper" => new VRML::NodeType("OrientationDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		value_changed => ["SFRotation", [0,1,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		initialValue => ["SFRotation", [0,1,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		set_destination => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFRotation", [0,1,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_input => ["SFRotation", [0,1,0,0], "initializeOnly", 0,0],#ff
	],"X3DDamperNode"),

	"PositionChaser" => new VRML::NodeType("PositionChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_steptime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		value_changed => ["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		initialValue => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		set_destination => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_previousvalue => ["SFVec3f", [0,0,0], "initializeOnly", 0,0],#ff
		_destination => ["SFVec3f", [0,0,0], "initializeOnly", 0,0],#ff
	],"X3DChaserNode"),
	
	"PositionDamper" => new VRML::NodeType("PositionDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		value_changed => ["SFVec3f", [0,0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		initialValue => ["SFVec3f", [0,0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		set_destination => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFVec3f", [0,0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_input => ["SFVec3f", [0,0,0], "initializeOnly", 0,0],#ff
	],"X3DDamperNode"),

	"PositionChaser2D" => new VRML::NodeType("PositionChaser2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_steptime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		value_changed => ["SFVec2f", [0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		initialValue => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		set_destination => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_previousvalue => ["SFVec2f", [0,0], "initializeOnly", 0,0],#ff
		_destination => ["SFVec2f", [0,0], "initializeOnly", 0,0],#ff
	],"X3DChaserNode"),
	
	"PositionDamper2D" => new VRML::NodeType("PositionDamper2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		value_changed => ["SFVec2f", [0,0], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		initialValue => ["SFVec2f", [0,0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		set_destination => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFVec2f", [0,0], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_input => ["SFVec2f", [0,0], "initializeOnly", 0,0],#ff
	],"X3DDamperNode"),

	"ScalarChaser" => new VRML::NodeType("ScalarChaser", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_steptime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		value_changed => ["SFFloat", 0, "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialValue => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_destination => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_previousvalue => ["SFFloat", 0, "initializeOnly", 0,0],#ff
		_destination => ["SFFloat", 0, "initializeOnly", 0,0],#ff
	],"X3DChaserNode"),
	
	"ScalarDamper" => new VRML::NodeType("ScalarDamper", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		value_changed => ["SFFloat", 0, "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialValue => ["SFFloat", 0, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_destination => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["SFFloat", 0, "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_input => ["SFFloat", 0, "initializeOnly", 0,0],#ff
	],"X3DDamperNode"),

	"TexCoordChaser2D" => new VRML::NodeType("TexCoordChaser2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		duration  => ["SFTime", 1,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_bufferendtime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_steptime  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		value_changed => ["MFVec2f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialValue => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_destination => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_buffer => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_previousvalue => ["MFVec2f", [[0,0]], "initializeOnly", 0,0],#ff
		_destination => ["MFVec2f", [[0,0]], "initializeOnly", 0,0],#ff
	],"X3DChaserNode"),
	
	"TexCoordDamper2D" => new VRML::NodeType("TexCoordDamper2D", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_p => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_t => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tolerance => ["SFFloat", -1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive  => ["SFBool", "FALSE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		order => ["SFInt32", 3, "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tau  => ["SFTime", .3,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_lasttick  => ["SFTime", 0,"initializeOnly", 0,0],#ff
		_takefirstinput => ["SFBool", "TRUE", "initializeOnly", 0,0],#ff
		value_changed => ["MFVec2f", [], "outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialDestination => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		initialValue => ["MFVec2f", [[0,0]], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_destination => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		set_value => ["MFVec2f", [], "inputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_values => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_input => ["MFVec2f", [], "initializeOnly", 0,0],#ff
	],"X3DDamperNode"),


	###################################################################################

	#	40.	Particle Systems Component

	###################################################################################
	#40.4.1 BoundedPhysicsModel
	"BoundedPhysicsModel" => new VRML::NodeType("BoundedPhysicsModel", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DParticlePhysicsModelNode"),
	
	# 40.4.2 ConeEmitter
	"ConeEmitter" => new VRML::NodeType("ConeEmitter", [
		angle  => ["SFFloat", "PIF*.25","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		position  => ["SFVec3f", [0,0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_AREA"],#ff
	],"X3DParticleEmitterNode"),
	
	# 40.4.3 ExplosionEmitter
	"ExplosionEmitter" => new VRML::NodeType("ExplosionEmitter", [
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		position  => ["SFVec3f", [0,0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_AREA"],#ff
	],"X3DParticleEmitterNode"),
	
	# 40.4.4 ForcePhysicsModel
	"ForcePhysicsModel" => new VRML::NodeType("ForcePhysicsModel", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		force => ["SFVec3f", [0,-9.8,0], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_FORCE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DParticlePhysicsModelNode"),
	
	
	# 40.4.5 ParticleSystem
	"ParticleSystem" => new VRML::NodeType ("ParticleSystem", [
		# shared with Shape, keep in same order as Shape:
		appearance => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
                metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_shaderflags_base =>["SFInt32",0,"initializeOnly",0,0],#ff # shaders
		_shaderflags_effects =>["SFInt32",0,"initializeOnly",0,0],#ff # shaders
		_shaderflags_usershaders =>["SFInt32",0,"initializeOnly",0,0],#ff # shaders
		# particlesystem specific:
		createParticles  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		lifetimeVariation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		maxParticles  => ["SFInt32", 200,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		particleLifetime  => ["SFFloat", 5,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		particleSize => ["SFVec2f", [.02,.02], "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		isActive  => ["SFBool", "TRUE","outputOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorRamp => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorKey => ["MFFloat", [], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		emitter => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		geometryType => ["SFString", "QUAD", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		physics => ["MFNode", [], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoordRamp => ["SFNode", "NULL", "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		texCoordKey => ["MFFloat", [], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_tris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_ttex => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_ltex => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_particles => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_lasttime => ["SFDouble", 0.0, "initializeOnly", 0,0],#ff
		_geometryType =>["SFInt32",0,"initializeOnly",0,0],#ff
		_remainder =>["SFFloat",0.0,"initializeOnly",0,0],#ff
	],"X3DShapeNode"),
	
	# 40.4.6 PointEmitter
	"PointEmitter" => new VRML::NodeType("PointEmitter", [
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		position  => ["SFVec3f", [0,0,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_AREA"],#ff
	],"X3DParticleEmitterNode"),
	
	# 40.4.7 PolylineEmitter
	"PolylineEmitter" => new VRML::NodeType("PolylineEmitter", [
		set_coordIndex => ["MFInt32", [], "inputOnly", "(SPEC_X3D33)","UNCA_NONE"],#ff
		set_coordinate => ["SFInt32", 0, "inputOnly", "(SPEC_X3D32)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		coordIndex => ["MFInt32", [-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_AREA"],#ff
		_method =>["SFInt32",0,"initializeOnly",0,0],#ff
		_nseg =>["SFInt32",0,"initializeOnly",0,0],#ff
		_segs => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		_portions => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
	],"X3DParticleEmitterNode"),
	
	# 40.4.8 SurfaceEmitter
	"SurfaceEmitter" => new VRML::NodeType("SurfaceEmitter", [
		set_coordIndex => ["MFInt32", [], "inputOnly", "( SPEC_X3D33)","UNCA_NONE"],#ff
		set_coordinate => ["SFInt32", 0, "inputOnly", "(SPEC_X3D32)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		coordIndex => ["MFInt32", [-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		surface => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_AREA"],#ff
		geometry => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_ifs => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
	],"X3DParticleEmitterNode"),
	
	# 40.4.9 VolumeEmitter
	"VolumeEmitter" => new VRML::NodeType("VolumeEmitter", [
		set_coordIndex => ["MFInt32", [], "inputOnly", "( SPEC_X3D33)","UNCA_NONE"],#ff
		set_coordinate => ["SFInt32", 0, "inputOnly", "(SPEC_X3D32)","UNCA_NONE"],#ff
		coord => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		speed  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		variation  => ["SFFloat", .25,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		coordIndex => ["MFInt32", [-1], "initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		internal  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		mass  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_MASS"],#ff
		surfaceArea  => ["SFFloat", 0,"initializeOnly", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_AREA"],#ff
		_ifs => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
	],"X3DParticleEmitterNode"),
	
	# 40.4.10 WindPhysicsModel
	"WindPhysicsModel" => new VRML::NodeType("WindPhysicsModel", [
		direction  => ["SFVec3f", [0,1,0],"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		gustiness => ["SFFloat", .1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		speed  => ["SFFloat", .1,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_SPEED"],#ff
		turbulence  => ["SFFloat", 0,"inputOutput", "(SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_frameSpeed =>["SFFloat",0.0,"initializeOnly",0,0],#ff
	],"X3DParticlePhysicsModelNode"),
	

	###################################################################################

	#	41.	Volume Rendering Component

	###################################################################################
	# LEVEL 1
	
	"OpacityMapVolumeStyle" => new VRML::NodeType("OpacityMapVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transferFunction => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	"VolumeData" => new VRML::NodeType("VolumeData", [
		dimensions  => ["SFVec3f", [1,1,1],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_boxtris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		renderStyle => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DVolumeDataNode"),


	#level 2
	# BoundaryEnhancementVolumeStyle	All fields fully supported.
	"BoundaryEnhancementVolumeStyle" => new VRML::NodeType("BoundaryEnhancementVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		boundaryOpacity => ["SFFloat", .9,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		opacityFactor => ["SFFloat", 2.0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		retainedOpacity => ["SFFloat", .2,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	# ComposedVolumeStyle	ordered field is always treated as FALSE. All other fields fully supported.
	"ComposedVolumeStyle" => new VRML::NodeType("ComposedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	# EdgeEnhancementVolumeStyle	All fields fully supported.
	"EdgeEnhancementVolumeStyle" => new VRML::NodeType("EdgeEnhancementVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		edgeColor=>["SFColorRGBA",[0,0,0,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		gradientThreshold => ["SFFloat", .4,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	# IsoSurfaceVolumeData	All fields fully supported.
	"IsoSurfaceVolumeData" => new VRML::NodeType("IsoSurfaceVolumeData", [
		dimensions  => ["SFVec3f", [1,1,1],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_boxtris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		contourStepSize => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		gradients => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceTolerance => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceValues => ["MFFloat",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff # see note top of file
	],"X3DVolumeDataNode"),
	
	# see level1: OpacityMapVolumeStyle	All fields fully supported. 3D transfer functions shall be supported.
	# ProjectionVolumeStyle	All fields fully supported
	"ProjectionVolumeStyle" => new VRML::NodeType("ProjectionVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		intensityThreshold => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		type => ["SFString", "MAX", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_type => ["SFInt32",0,"initializeOnly",0,0],#ff		
	],"X3DComposableVolumeRenderStyleNode"),
	
	# SegmentedVolumeData	All fields fully supported.
	"SegmentedVolumeData" => new VRML::NodeType("SegmentedVolumeData", [
		dimensions  => ["SFVec3f", [1,1,1],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_LENGTH"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bboxCenter => ["SFVec3f", [0, 0, 0], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		bboxSize => ["SFVec3f", [-1, -1, -1], "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_BLENGTH"],#ff
		_boxtris => ["FreeWRLPTR", "NULL", "initializeOnly", 0,0],#ff
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		segmentEnabled => ["MFBool",[],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff  # see note top of file
		segmentIdentifiers => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DVolumeDataNode"),
	
	# SilhouetteEnhancementVolumeStyle	All fields fully supported.
	"SilhouetteEnhancementVolumeStyle" => new VRML::NodeType("SilhouetteEnhancementVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		silhouetteBoundaryOpacity => ["SFFloat", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		silhouetteRetainedOpacity => ["SFFloat", 1,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		silhouetteSharpness => ["SFFloat", .5,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	
	# ToneMappedVolumeStyle	All fields fully supported.
	"ToneMappedVolumeStyle" => new VRML::NodeType("ToneMappedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		coolColor=>["SFColorRGBA",[0,0,1,0],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		warmColor=>["SFColorRGBA",[1,1,0,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	#level 3
	# BlendedVolumeStyle	All fields fully supported.
	"BlendedVolumeStyle" => new VRML::NodeType("BlendedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		renderStyle => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		voxels => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weightConstant1 => ["SFFloat", 0.5,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weightConstant2 => ["SFFloat", 0.5,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weightFunction1 => ["SFString", "CONSTANT", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weightFunction2 => ["SFString", "CONSTANT", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weightTransferFunction1 => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		weightTransferFunction2 => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_fbohandles => ["MFInt32",[0,0,0],"initializeOnly",0,0],#ff
		_weightFunction1 => ["SFInt32",0,"initializeOnly",0,0],#ff
		_weightFunction2 => ["SFInt32",0,"initializeOnly",0,0],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	# CartoonVolumeStyle	All fields fully supported.
	"CartoonVolumeStyle" => new VRML::NodeType("CartoonVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		orthogonalColor=>["SFColorRGBA",[1,1,1,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		parallelColor=>["SFColorRGBA",[0,0,0,1],"inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		colorSteps => ["SFInt32", 4, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	# CompositeVolumeStyle	All fields fully supported.
	"CompositeVolumeStyle" => new VRML::NodeType("CompositeVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		#the other renderStyles are SF, this one MF
		renderStyle => ["MFNode", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DComposableVolumeRenderStyleNode"),
	
	# ShadedVolumeStyle	All fields fully supported except shadows. Shadows supported with at least Phong shading.
	#level 4
	# ShadedVolumeStyle	All fields fully supported with at least Phong shading and  Henyey-Greenstein phase function. Shadows fully supported.
	"ShadedVolumeStyle" => new VRML::NodeType("ShadedVolumeStyle", [
		enabled  => ["SFBool", "TRUE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		surfaceNormals => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		lighting  => ["SFBool", "FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		shadows => ["SFBool", "FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		material => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		phaseFunction => ["SFString", "Henyey-Greenstein", "initializeOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		_phaseFunction => ["SFInt32",0,"initializeOnly",0,0],#ff
	],"X3DComposableVolumeRenderStyleNode"),


	###################################################################################

	# Augmented Reality - not in specs, proposed:
	# http://www.web3d.org/wiki/index.php?title=AR_Proposal_Public_Review
	
	###################################################################################

	"BackdropBackground" => new VRML::NodeType("BackdropBackground", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transparency => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFColor", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__texture => ["SFInt32", 0, "inputOutput", 0,0],#ff
		__VBO=>["SFInt32",0,"initializeOnly",0,0],#ff  # Vertex Buffer Object, if required.
		url => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DBackgroundNode"),
	
	"ImageBackdropBackground" => new VRML::NodeType("ImageBackdropBackground", [
		set_bind => ["SFBool", 100, "inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		bindTime => ["SFTime",0,"outputOnly", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isBound => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		transparency => ["SFFloat", 0.0, "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		color => ["SFColor", [0,0,0], "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		__texture => ["SFInt32", 0, "inputOutput", 0,0],#ff
		__VBO=>["SFInt32",0,"initializeOnly",0,0],#ff  # Vertex Buffer Object, if required.
		image => ["SFImage", "0, 0, 0", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DBackgroundNode"),

	"CalibratedCameraSensor" => new VRML::NodeType("CalibratedCameraSensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		image => ["SFImage", "0, 0, 0", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		focalPoint => ["SFVec2f", [0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fieldOfView => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		fovMode => ["SFString", "", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		aspectRatio => ["SFFloat", 0.0, "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DSensorNode"),
	
	"TrackingSensor" => new VRML::NodeType("TrackingSensor", [
		enabled => ["SFBool", "TRUE", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		position => ["SFVec3f", [0, 0, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		rotation => ["SFRotation", [0, 0, 1, 0], "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isActive => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isPositionAvailable => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		isRotationAvailable => ["SFBool", "FALSE", "outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
	],"X3DSensorNode"),


	# Metadata nodes ...

	###################################################################################

	#used mainly for (pre-2014 era text-based PROTOs attached to Group nodes aka TROTO) PROTO invocation parameters
	#(2014+ era: switched to binary PROTOs (aka Brotos) with their own (not Group) node, which uses routing to go from 
	#  BrotoInterface to BrotoBody nodes - don't need the following now, or the __protoDEF thing in Group, 
	#   except to compile left-over code)
	"MetadataSFFloat" => new VRML::NodeType("MetadataSFFloat", [
		value => ["SFFloat",0.0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFFloat",0.0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFFloat",0.0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFFloat" => new VRML::NodeType("MetadataMFFloat", [
		value => ["MFFloat",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFFloat",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFFloat",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFRotation" => new VRML::NodeType("MetadataSFRotation", [
		value => ["SFRotation",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		valueChanged=>["SFRotation",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFRotation",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFRotation" => new VRML::NodeType("MetadataMFRotation", [
		value => ["MFRotation",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_ANGLE"],#ff
		valueChanged=>["MFRotation",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFRotation",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec3f" => new VRML::NodeType("MetadataSFVec3f", [
		value => ["SFVec3f",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFVec3f",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFVec3f",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec3f" => new VRML::NodeType("MetadataMFVec3f", [
		value => ["MFVec3f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFVec3f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFVec3f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFBool" => new VRML::NodeType("MetadataSFBool", [
		value => ["SFBool","FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFBool","FALSE","outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFBool","FALSE","inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFBool" => new VRML::NodeType("MetadataMFBool", [
		value => ["MFBool",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFBool",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFBool",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFInt32" => new VRML::NodeType("MetadataSFInt32", [
		value => ["SFInt32",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFInt32",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFInt32",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFInt32" => new VRML::NodeType("MetadataMFInt32", [
		value => ["MFInt32",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFInt32",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFInt32",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFNode" => new VRML::NodeType("MetadataSFNode", [
		value => ["SFNode",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFNode",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFNode",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFNode" => new VRML::NodeType("MetadataMFNode", [
		value => ["MFNode",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFNode",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFNode",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFColor" => new VRML::NodeType("MetadataSFColor", [
		value => ["SFColor",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFColor",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFColor",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFColor" => new VRML::NodeType("MetadataMFColor", [
		value => ["MFColor",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFColor",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFColor",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFColorRGBA" => new VRML::NodeType("MetadataSFColorRGBA", [
		value => ["SFColorRGBA",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFColorRGBA",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFColorRGBA",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFColorRGBA" => new VRML::NodeType("MetadataMFColorRGBA", [
		value => ["MFColorRGBA",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFColorRGBA",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFColorRGBA",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFTime" => new VRML::NodeType("MetadataSFTime", [
		value => ["SFTime",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFTime",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFTime",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFTime" => new VRML::NodeType("MetadataMFTime", [
		value => ["MFTime",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFTime",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFTime",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFString" => new VRML::NodeType("MetadataSFString", [
		value => ["SFString","","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFString","","outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFString","","inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFString" => new VRML::NodeType("MetadataMFString", [
		value => ["MFString",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFString",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFString",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec2f" => new VRML::NodeType("MetadataSFVec2f", [
		value => ["SFVec2f",[0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFVec2f",[0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFVec2f",[0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec2f" => new VRML::NodeType("MetadataMFVec2f", [
		value => ["MFVec2f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFVec2f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFVec2f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFImage" => new VRML::NodeType("MetadataSFImage", [
		value => ["SFImage",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFImage",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFImage",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec3d" => new VRML::NodeType("MetadataSFVec3d", [
		value => ["SFVec3d",[0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFVec3d",[0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFVec3d",[0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec3d" => new VRML::NodeType("MetadataMFVec3d", [
		value => ["MFVec3d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFVec3d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFVec3d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFDouble" => new VRML::NodeType("MetadataSFDouble", [
		value => ["SFDouble",0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFDouble",0,"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFDouble",0,"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFDouble" => new VRML::NodeType("MetadataMFDouble", [
		value => ["MFDouble",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFDouble",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFDouble",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix3f" => new VRML::NodeType("MetadataSFMatrix3f", [
		value => ["SFMatrix3f",[0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFMatrix3f",[0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFMatrix3f",[0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix3f" => new VRML::NodeType("MetadataMFMatrix3f", [
		value => ["MFMatrix3f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFMatrix3f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFMatrix3f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix3d" => new VRML::NodeType("MetadataSFMatrix3d", [
		value => ["SFMatrix3d",[0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFMatrix3d",[0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFMatrix3d",[0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix3d" => new VRML::NodeType("MetadataMFMatrix3d", [
		value => ["MFMatrix3d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFMatrix3d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFMatrix3d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix4f" => new VRML::NodeType("MetadataSFMatrix4f", [
		value => ["SFMatrix4f",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFMatrix4f",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFMatrix4f",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix4f" => new VRML::NodeType("MetadataMFMatrix4f", [
		value => ["MFMatrix4f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFMatrix4f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFMatrix4f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFMatrix4d" => new VRML::NodeType("MetadataSFMatrix4d", [
		value => ["SFMatrix4d",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFMatrix4d",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFMatrix4d",[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFMatrix4d" => new VRML::NodeType("MetadataMFMatrix4d", [
		value => ["MFMatrix4d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFMatrix4d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFMatrix4d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec2d" => new VRML::NodeType("MetadataSFVec2d", [
		value => ["SFVec2d",[0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFVec2d",[0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFVec2d",[0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec2d" => new VRML::NodeType("MetadataMFVec2d", [
		value => ["MFVec2d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFVec2d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFVec2d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec4f" => new VRML::NodeType("MetadataSFVec4f", [
		value => ["SFVec4f",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFVec4f",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFVec4f",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec4f" => new VRML::NodeType("MetadataMFVec4f", [
		value => ["MFVec4f",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFVec4f",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFVec4f",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataSFVec4d" => new VRML::NodeType("MetadataSFVec4d", [
		value => ["SFVec4d",[0,0,0,0],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["SFVec4d",[0,0,0,0],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["SFVec4d",[0,0,0,0],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
	], "X3DChildNode"),

	#used mainly for PROTO invocation parameters
	"MetadataMFVec4d" => new VRML::NodeType("MetadataMFVec4d", [
		value => ["MFVec4d",[],"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		valueChanged=>["MFVec4d",[],"outputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		setValue =>["MFVec4d",[],"inputOnly", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		tickTime=>["SFTime",0,"inputOnly",0,0],#ff
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

		enabled => ["SFBool", "FALSE","inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		description => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		protocol => ["SFString", "UDP", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		listenfor => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		port => ["SFInt32", 7000, "inputOutput", 0,0],#ff
		filter => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		handler => ["SFString", "", "inputOutput", "(SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		talksTo => ["MFString", [], "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		FIFOsize  => ["SFInt32", 64, "inputOutput",, 0,0],#ff
		int32Inp => ["SFInt32", 0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		floatInp => ["SFFloat", 0.0, "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		stringInp => ["SFString", "", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff
		gotEvents => ["SFInt32", 0,"inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		metadata => ["SFNode", "NULL", "inputOutput", "(SPEC_VRML | SPEC_X3D30 | SPEC_X3D31 | SPEC_X3D32 | SPEC_X3D33)","UNCA_NONE"],#ff

		_talkToNodes => ["MFNode", [], "inputOutput", 0,0],#ff
		_status => ["SFInt32", -1, "inputOutput", 0,0],#ff
		_int32InpFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_floatInpFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_stringInpFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_int32OutFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_floatOutFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		_stringOutFIFO => ["FreeWRLPTR",0,"initializeOnly", 0,0],#ff
		__oldmetadata => ["SFNode", 0, "inputOutput", 0,0],#ff # see code for event macro

	],"X3DNetworkSensorNode"),




);


1;

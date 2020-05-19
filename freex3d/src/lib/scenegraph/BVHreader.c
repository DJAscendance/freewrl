

// license: MIT or similar permissive
/*
BVH includes some skeleton information X3D HAnim doesn't need / can't use:
x length of bones aka OFFSET
* HAnim supplies its own skeleton 
x parenting heirarchy to help accumulate global transform for a limb
* HAnim - we do transforms at each joint, so we need only local joint angles

What X3D needs is HAnim2MotionData.

*/


#define ASSERT

#include <config.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include "LinearAlgebra.h"

#define TRUE 1
#define FALSE 0
#define NULL ((void *)0)

#ifdef _MSC_VER
#define strcasecmp stricmp
#endif //_MSC_VER
#define RADIANS_PER_DEGREE (double)0.0174532925199432957692
#define DEGREES_PER_RADIAN (double)57.2957795130823208768


enum {
CHAN_RX = 1,
CHAN_RY = 2,
CHAN_RZ = 3,
CHAN_TX = 4,
CHAN_TY = 5,
CHAN_TZ = 6,
CHAN_NONE = 0,
};
static struct chan_name {
int iname;
char *cname;
} chan_names [] = {
{CHAN_RX, "Xrotation"},
{CHAN_RY, "Yrotation"},
{CHAN_RZ, "Zrotation"},
{CHAN_TX, "Xposition"},
{CHAN_TY, "Yposition"},
{CHAN_TZ, "Zposition"},
{CHAN_NONE,NULL},
};
static int chan_lookup(char *cname){
	int i, iname;
	struct chan_name *cn;
	i = 0;
	iname = 0;
	do{
		cn = &chan_names[i];
		if(!strcmp(cn->cname,cname)){
			iname = cn->iname;
			break;
		}
		i++;
	}while(cn->cname != NULL);
	return iname;
	
}

char * getline(char *line, int maxlen, char **position){
	char *cur = *position;
	char *end = strstr(cur,"\n");
	if(end == NULL) return NULL;
	int len = min(end-cur,maxlen-1);
	memcpy(line,cur,len);
	line[len] = '\0';
	*position = &cur[len+1];
	return *position;
}

struct joint_frame_motion {
	char *jname;
	char *mocap_name;
	int nchan;
	int ichan[6];
	float *values;
};
void read_bvh_blob(char *blob, struct joint_frame_motion **chan, int *njoint, int *channel_count, float **values, float *bvh_frame_time, int *bvh_frame_count);
void read_bvh_blob(char *blob, struct joint_frame_motion **chan, int *njoint, int *channel_count, float **values, float *bvh_frame_time, int *bvh_frame_count)
{
    // File loading stuff
    // Open the file for importing

    // Seperate into a list of lists, each line a list of words.
	char *rv; 
	char line [4096], *pos;
	char *token, *delims;
	float global_scale = 1.0f;

	pos = blob;
	rv = getline(line,2048,&pos);


    // Split by whitespace.
	delims = " ,\r\n\t\"";
	token = strtok(line,delims);
    // Create hierarchy as empties
	if( strcasecmp(token,"hierarchy")){
		printf("not a BVH file \n");
		return;
	}

    *bvh_frame_count = 0;
    *bvh_frame_time = 0.0;

	int channelIndex = -1;
	*channel_count = 0;
	struct joint_frame_motion *cjoint, *cj, ccjoints[100];
	cjoint = ccjoints; //malloc(100 * sizeof(struct joint_frame_motion));
	memset(cjoint,0,100*sizeof(struct joint_frame_motion));
	int mjoint = 0;
	while( getline(line,2048,&pos)){
        //...
		token = strtok(line,delims);
		printf("token %s\n",token);
        if(!strcasecmp(token,"root") || !strcasecmp(token,"joint")){
			// JOINT name, start new joint
			char *nametokens[4];
			char name[100];
			int len=0;
			memset(nametokens,0,4*sizeof(void*));
			while(nametokens[len] = strtok(NULL,delims)) len++;
			printf("len %d\n",len);
            // Join spaces into 1 word with underscores joining it.
			strcpy(name,nametokens[0]);
			printf("name=%s\n",name);
            // Make sure the names are unique - Object names will match joint names exactly and both will be unique.
			for(int i=1;i<len-1;i++) {
				strcat(name,"_");
				strcat(name,nametokens[i]);
			}
			cj = &cjoint[mjoint];
			mjoint++;
			cj->mocap_name = strdup(name);
		}
		if(!strcasecmp(token,"OFFSET")){
			if(0){
				//get offset numbers
				float offset[3];
				for(int i=0;i<3;i++){
					token = strtok(NULL,delims);
					sscanf(token,"%f",&offset[i]);
				}
			}
		}
		if(!strcasecmp(token,"CHANNELS")){
 			int channels;
			token = strtok(NULL,delims); //CHANNELS
			sscanf(token,"%d",&channels);
			*channel_count += channels;
			cj->nchan = channels;

            //for channel in file_lines[lineIdx][2:]:
			for(int i=0;i<channels;i++){
				char *channel = strtok(NULL,delims); //Zrotation
				int ichan = chan_lookup(channel);
				cj->ichan[i] = ichan;

 			}
 		}	
        if(!strcasecmp(token,"end")){
		}
		if(!strcmp(token,"}")){
		}
		if(!strcmp(token,"{")){
		}
        if(!strcasecmp(token,"motion") ){ //MOTION
	        // End of joint hierarchy.
	        //  MOTION
            break; //get out of hierarchy loop 
		}
	} //end while lines

	*njoint = mjoint;
    // start of motion channel float values, starting with:
    //  Frames: n
    //  Frame Time: dt
	getline(line,2048,&pos); //Frames:	2752
	token = strtok(line,delims); //frames:
	if(!strcasecmp(token,"frames:")){
		token = strtok(NULL,delims); //2752
		sscanf(token,"%d",bvh_frame_count);
	}

	getline(line,2048,&pos); //Frame Time:	0.00833333
	token = strtok(line,delims); //frame
	if(!strcasecmp(token,"frame")){
		token = strtok(NULL,delims); //time
		if(!strcasecmp(token,"time:")){
			token = strtok(NULL,delims); //0.00833333
			sscanf(token,"%f",bvh_frame_time);
		}
	}

	printf("njoint %d \n",*njoint);
	struct joint_frame_motion *cchan = malloc(*njoint *sizeof(struct joint_frame_motion));
	*chan = cchan;
	memcpy(cchan,cjoint,mjoint * sizeof(struct joint_frame_motion));
 
	float * fvalues = malloc( (*channel_count) * (*bvh_frame_count) * sizeof(float));
	*values = fvalues;
	int k = 0;
	//char *delims2 = " ,\t\r\n";
	char *str = pos;
	for(int iframe=0;iframe<*bvh_frame_count;iframe++){
        for(int i=0;i<*channel_count;i++){
			token = strtok(str,delims);
			str = NULL; //so next strtok(NULL,...)
			sscanf(token,"%f",&fvalues[k]);
			k++;
		}
	}

	//convert degrees to radians
	for(int iframe=0;iframe< *bvh_frame_count;iframe++){
		float *fv = &fvalues[iframe * (*channel_count)];
		int kchan = 0;
		for(int j=0;j<mjoint;j++){
			//printf("%s %d \n",vector_get(char*,jnames,j),chan[j].nchan);
			for(int k=0;k<cchan[j].nchan;k++){
				if(cchan[j].ichan[k] < 4)
					fv[kchan] *= RADIANS_PER_DEGREE; //PI / 180.0; //
				//printf("%d %5.2f ",chan[j].ichan[k],chan[j].ichan[k] < 4 ? fv[kchan]*180.0/PI : fv[kchan]);
				kchan++;
			}
			//printf("\n");
		}
	}

}


// Mapping no	LOA-1 HAnim joints (18 joints)	motion-capture joints example (18 different joints)
struct name_map {
int no;
char *jname;
char *mocap_name[5];
} loa1_mapping [] = {
{1,"humanoid_root",{"Hips","hip",0,0,0}},
{2,"sacroiliac",{0,0,0,0,0}},
{3,"l_hip",{"LeftHip","lThigh",0,0,0}},
{4,"l_knee",{"LeftKnee","lShin",0,0,0}},
{5,"l_talocrural",{"LeftAnkle","lFoot",0,0,0}},
{6,"l_metatarsophalangeal",{0,0,0,0,0}},
{7,"r_hip",{"RightHip","rThigh",0,0,0}},
{8,"r_knee",{"RightKnee","rShin",0,0,0}},
{9,"r_talocrural",{"RightAnkle","rFoot",0,0,0}},
{10,"r_metatarsophalangeal",{0,0,0,0,0}},
{11,"vl5",{"Chest","abdomen",0,0,0}},
{12,"skullbase",{"Neck","Head",0,0,0}},
{13,"l_shoulder",{"LeftCollar","lCollar",0,0,0}},
{13,"l_shoulder",{"LeftShoulder","lShldr",0,0,0}},
{14,"l_elbow",{"LeftElbow","lForeArm",0,0,0}},
{15,"l_radiocarpal",{"LeftWrist","lHand",0,0,0}},
{16,"r_shoulder",{"RightCollar","rCollar",0,0,0}},
{16,"r_shoulder",{"RightShoulder","rShldr",0,0,0}},
{17,"r_elbow",{"RightElbow","rForeArm",0,0,0}},
{18,"r_radiocarpal",{"RightWrist","rHand",0,0,0}},
{0,NULL,{0,0,0,0,0}},
};
static char *ignore = "IGNORE";
char * jname_lookup(char *mocap_name){
	int i, iname;
	char *jname = ignore;
	struct name_map *nm;
	i = 0;
	iname = -1;
	do{
		nm = &loa1_mapping[i];
		int j=0;
		char *nm_mocap_name;
		while(nm_mocap_name = nm->mocap_name[j]){
			if(!strcasecmp(nm_mocap_name,mocap_name)){
				//great built-in mapping!
				iname = i;
				break;
			}
			j++;
		}
		if(iname > -1) break;
		i++;
	}while(nm->no > 0);
	if(iname > -1){
		jname = loa1_mapping[iname].jname;
	}
	return jname;
}


void map_mocap_to_hanim_loa( struct joint_frame_motion *chan, int mjoint, int loa){

	//map mocap joint names to HAnim2 loa joint names - see section 4.4.4 Joint mapping example
	printf("=====BEFORE MAPPING====\n");
	for(int i=0;i<mjoint;i++){
		//printf("%d %s\n",i,cjoint[i].mocap_name);
		printf("%d %s\n",i,chan[i].mocap_name);
	}
	printf("=========\n");

	if(loa == 1){
		for(int i=0;i<mjoint;i++){
			chan[i].jname = jname_lookup(chan[i].mocap_name);
		}
	}

	printf("====AFTER MAPPING=====\n");
	for(int i=0;i<mjoint;i++){
		//printf("%d %s\n",i,cjoint[i].mocap_name);
		printf("%d %s\n",i,chan[i].jname);
	}
	printf("=========\n");
}


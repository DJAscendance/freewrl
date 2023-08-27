

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
#include <stdio.h>
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

char * getline2(char *line, int maxlen, char **position){
	char *cur = *position;
	char *end = strstr(cur,"\n");
	if(end == NULL) return NULL;
	int len = (end-cur) < (maxlen-1) ? (end-cur) : (maxlen-1);
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
char* get_jname(char* mocap_name);
static char* swaplistleft[] = { "l_shoulder" ,"l_elbow", "l_wrist", NULL, };
static char* swaplistright[] = { "r_shoulder", "r_elbow", "r_wrist", NULL, };
static int instringlist(char* name, char** list) {
	int have = FALSE;
	int i = 0;
	while (list[i]) {
		if (!strcmp(name, list[i])) {
			have = TRUE; break;
		}
		i++;
	}
	return have;
}

void read_bvh_blob(char* blob, int ignorePosition, int yUp, int teePose,
	int flipZ, float armAngle, float legAngle, float scale,
	struct joint_frame_motion** chan, int* njoint, int* channel_count, float** values,
	float* bvh_frame_time, int* bvh_frame_count);
void read_bvh_blob(char* blob, int ignorePosition, int yUp, int teePose,
	int flipZ, float armAngle, float legAngle, float scale,
	struct joint_frame_motion** chan, int* njoint, int* channel_count, float** values,
	float* bvh_frame_time, int* bvh_frame_count)
{
    // File loading stuff
    // Open the file for importing

    // Seperate into a list of lists, each line a list of words.
	char *rv; 
	char line [4096], *pos;
	char *token, *delims;
	float global_scale = 1.0f;

	pos = blob;
	rv = getline2(line,2048,&pos);


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
	while( getline2(line,2048,&pos)){
        //...
		token = strtok(line,delims);
		//printf("token %s\n",token);
        if(!strcasecmp(token,"root") || !strcasecmp(token,"joint")){
			// JOINT name, start new joint
			char *nametokens[4];
			char name[100];
			int len=0;
			memset(nametokens,0,4*sizeof(void*));
			while(nametokens[len] = strtok(NULL,delims)) len++;
			//printf("len %d\n",len);
            // Join spaces into 1 word with underscores joining it.
			strcpy(name,nametokens[0]);
			printf("%d name=%s ",mjoint, name);
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
					offset[i] *= scale;
				}
			}
		}
		if(!strcasecmp(token,"CHANNELS")){
 			int channels;
			token = strtok(NULL,delims); //CHANNELS
			sscanf(token,"%d",&channels);
			*channel_count += channels;
			printf(" channels %d totalchannels %d\n",channels,*channel_count );
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
	getline2(line,2048,&pos); //Frames:	2752
	token = strtok(line,delims); //frames:
	if(!strcasecmp(token,"frames:")){
		token = strtok(NULL,delims); //2752
		sscanf(token,"%d",bvh_frame_count);
	}

	getline2(line,2048,&pos); //Frame Time:	0.00833333
	token = strtok(line,delims); //frame
	if(!strcasecmp(token,"frame")){
		token = strtok(NULL,delims); //time
		if(!strcasecmp(token,"time:")){
			token = strtok(NULL,delims); //0.00833333
			sscanf(token,"%f",bvh_frame_time);
		}
	}

	printf("njoint %d \n",*njoint);
	printf("nchannel %d\n",*channel_count);
	struct joint_frame_motion *cchan = malloc(*njoint *sizeof(struct joint_frame_motion));
	*chan = cchan;
	memcpy(cchan,cjoint,mjoint * sizeof(struct joint_frame_motion));
 
	float * fvalues = malloc( (*channel_count) * (*bvh_frame_count) * sizeof(float));
	*values = fvalues;
	int k = 0;
	//char *delims2 = " ,\t\r\n";
	char *str = pos;
	//FILE * fout = fopen("single_row.bvh","w+");
	for(int iframe=0;iframe<*bvh_frame_count;iframe++){
        for(int i=0;i<*channel_count;i++){
			token = strtok(str,delims);
			str = NULL; //so next strtok(NULL,...)
			sscanf(token,"%f",&fvalues[k]);
			//fprintf(fout,"%f ",fvalues[k]);
			k++;
		}
		//fprintf(fout,"\n");
	}
	//fclose(fout);
	float* fv0 = &fvalues[0];

	//convert degrees to radians
	for(int iframe=0;iframe< *bvh_frame_count;iframe++){
		float *fv = &fvalues[iframe * (*channel_count)];
		int kchan = 0;
		for(int j=0;j<mjoint;j++){
			//printf("%s %d \n",vector_get(char*,jnames,j),chan[j].nchan);
			int axis_swap_left, axis_swap_right;
			char* jname = get_jname(cchan[j].mocap_name);
			axis_swap_left = axis_swap_right = 0;
			if (teePose) {
				axis_swap_left = instringlist(jname, swaplistleft);
				axis_swap_right = instringlist(jname, swaplistright);
				if (axis_swap_left || axis_swap_right) {
					int ix, iy;
					for (int k = 0; k < cchan[j].nchan; k++) {
						if (cchan[j].ichan[k] == 1) ix = k;
						if (cchan[j].ichan[k] == 2) iy = k;
					}
					if (axis_swap_left) {
						float tmp = fv[kchan + ix];
						fv[kchan + ix] = fv[kchan + iy];
						fv[kchan + iy] = -tmp;
					}
					if (axis_swap_right) {
						float tmp = fv[kchan + ix];
						fv[kchan + ix] = -fv[kchan + iy];
						fv[kchan + iy] = tmp;
					}
				}
			}

			for(int k=0;k<cchan[j].nchan;k++){
				if (cchan[j].ichan[k] < 4) {
					if (flipZ) {
						if (cchan[j].ichan[k] == 3)
							fv[kchan] *= -1;
					}
					if (legAngle != 0.0f) {
						if (cchan[j].ichan[k] == 3 && !strcmp(jname, "l_hip"))
							fv[kchan] += legAngle;
						else if (cchan[j].ichan[k] == 3 && !strcmp(jname, "r_hip"))
							fv[kchan] -= legAngle;
					}
					if(armAngle != 0.0f){
						if (cchan[j].ichan[k] == 3 && !strcmp(jname, "l_shoulder"))
							fv[kchan] += armAngle;
						else if (cchan[j].ichan[k] == 3 && !strcmp(jname, "r_shoulder"))
							fv[kchan] -= armAngle;
					}
					fv[kchan] *= RADIANS_PER_DEGREE; //PI / 180.0; //
				}
				if (cchan[j].ichan[k] > 3) {
					fv[kchan] *= scale;
					if (cchan[j].ichan[k] == 6 && flipZ)
						fv[kchan] = -fv[kchan];
					if (ignorePosition)
						fv[kchan] = 0.0f;
				}
				//if(!strcmp(get_jname(cchan[j].mocap_name),"l_shoulder"))
				//	printf("%d %5.2f ",cchan[j].ichan[k],cchan[j].ichan[k] < 4 ? fv[kchan]*DEGREES_PER_RADIAN : fv[kchan]);
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
char *mocap_name[6];
} loa1_mapping [] = {
{1,"humanoid_root",{"Hips","hip","joint_root",0,0}},
{2,"sacroiliac",{"Spine",0,0,0,0,0}},
{3,"l_hip",{"LeftHip","lThigh","UpperLeg_L","LeftUpLeg",0,0}},
{4,"l_knee",{"LeftKnee","lShin","LowerLeg_L","LeftLeg",0,0}},
{5,"l_talocrural",{"LeftAnkle","lFoot","Foot_L","LeftFoot",0,0}},
{6,"l_metatarsophalangeal",{"Toes_L","LeftToeBase",0,0,0,0}},
{7,"r_hip",{"RightHip","rThigh","UpperLeg_R","RightUpLeg",0,0}},
{8,"r_knee",{"RightKnee","rShin","LowerLeg_R","RightLeg",0,0}},
{9,"r_talocrural",{"RightAnkle","rFoot","Foot_R","RightFoot",0,0}},
{10,"r_metatarsophalangeal",{"Toes_R","RightToeBase",0,0,0,0}},
{11,"vl5",{"Chest","abdomen","Spine1",0,0,0}},
{12,"skullbase",{"Neck","Head",0,0,0,0}},
//{13,"l_shoulder",{"LeftCollar","lCollar",0,0,0,0}},
{13,"l_shoulder",{"LeftShoulder","lShldr","UpperArm_L","LeftArm",0,0}},
{14,"l_elbow",{"LeftElbow","lForeArm","LowerArm_L","LeftForeArm",0,0}},
{15,"l_radiocarpal",{"LeftWrist","lHand","Hand_L","LeftHand",0,0}},
//{16,"r_shoulder",{"RightCollar","rCollar",0,0,0,0}},
{16,"r_shoulder",{"RightShoulder","rShldr","UpperArm_R","RightArm",0,0}},
{17,"r_elbow",{"RightElbow","rForeArm","LowerArm_R","RightForeArm",0,0}},
{18,"r_radiocarpal",{"RightWrist","rHand","Hand_R","RightHand",0,0}},
{0,NULL,{0,0,0,0,0,0}},
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

//every bvh publisher and bvh seems to have different naming convention
//if you worked out a mapping from bvh joint name to LOA1 joint name
// set it here and we'll use it
static char** mapp = NULL;
static int nmap = 0;
void bvh_set_mapping(char** mapping, int n) {
	mapp = mapping;
	nmap = n;
}
char* jname_mapping(char* mocap_name) {
	char* jname = ignore;
	for (int i = 0; i < nmap; i++) {
		if (!strcasecmp(mapp[i * 2 + 1], mocap_name)) {
			jname = mapp[i * 2];
		}
	}
	return jname;
}
char* get_jname(char* mocap_name) {
	char* jname = NULL;
	if (mapp) {
		jname = jname_mapping(mocap_name);
	}
	else {
		jname = jname_lookup(mocap_name);
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

	if(loa == 1 || loa == -1){
		for(int i=0;i<mjoint;i++){
			chan[i].jname = get_jname(chan[i].mocap_name);
			//if (mapp) {
			//	chan[i].jname = jname_mapping(chan[i].mocap_name);
			//}
			//else {
			//	chan[i].jname = jname_lookup(chan[i].mocap_name);
			//}
		}
	}

	printf("====AFTER MAPPING=====\n");
	for(int i=0;i<mjoint;i++){
		//printf("%d %s\n",i,cjoint[i].mocap_name);
		printf("%d %s\n",i,chan[i].jname);
	}
	printf("=========\n");
}


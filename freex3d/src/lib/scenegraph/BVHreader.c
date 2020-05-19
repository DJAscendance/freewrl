

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
#include <system.h>
#include "Vector.h"
#include "LinearAlgebra.h"

// .H  >> 

void read_bvh(char *file_path, float global_scale,
	Stack *bvh_nodes, float *bvh_frame_time, int *bvh_frame_count);

// << .H


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



#define TRUE 1
#define FALSE 0
struct eul_order {
	int order[3];
	char *str;
} eul_orders [] = {
	{-1,-1,-1,"XYZ"},  // XXX Dummy one, no rotation anyway!
    {0, 1, 2, "XYZ"},
    {0, 2, 1, "XZY"},
    {1, 0, 2, "YXZ"},
    {1, 2, 0, "YZX"},
    {2, 0, 1, "ZXY"},
    {2, 1, 0, "ZYX"},
};
char * eul_order_lookup (int *order) {
	for(int i=0;i<7;i++){
		int match = TRUE;
		for(int j=0;j<3;j++)
			match = match && order[j] == eul_orders[i].order[j];
		if(match){
			return eul_orders[i].str;
		}
	}
	return eul_orders[0].str;
}

struct anim_record{
	float lxyz[3];
	float rxyz[3];
};


struct BVH_Node {
	char *name; // bvh joint name
	struct BVH_Node *parent;  // BVH_Node type or None for no parent
	Stack * children;  // a list of children of this type.
    float rest_head_world[3];  // worldspace rest location for the head of this node
    float rest_head_local[3];   // localspace rest location for the head of this node
    float rest_tail_world[3];  // worldspace rest location for the tail of this node
    float rest_tail_local[3];  // worldspace rest location for the tail of this node
	float rest_tail_local_store[3];
	int channels[7];  // list of 6 ints, -1 for an unused channel, otherwise an index for the BVH motion data lines, loc triple then rot triple
	int rot_order[3];  // a triple of indices as to the order rotation is applied. [0,1,2] is x/y/z - [None, None, None] if no rotation.
    char * rot_order_str; // same as above but a string 'XYZ' format.
    Stack *anim_data; //[6];  // a list one tuple's one for each frame. (locx, locy, locz, rotx, roty, rotz), euler rotation ALWAYS stored xyz order, even when native used.
    BOOL has_loc;  // Convenience function, bool, same as (channels[0]!=-1 or channels[1]!=-1 or channels[2]!=-1)
    BOOL has_rot;  // Convenience function, bool, same as (channels[3]!=-1 or channels[4]!=-1 or channels[5]!=-1)
    int index;  // index from the file, not strictly needed but nice to maintain order
    float *temp;  // use this for whatever you want
};

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
struct BVH_Node * init_BVH_Node( char *name, float * rest_head_world, float * rest_head_local, 
	struct BVH_Node *parent, int *channels, int *rot_order, int index){
	struct BVH_Node *self = (struct BVH_Node*)malloc(sizeof(struct BVH_Node));
	memset(self,0,sizeof(struct BVH_Node));

	self->name = name;
	veccopy3f(self->rest_head_world,rest_head_world);
	veccopy3f(self->rest_head_local,rest_head_local);
	vecset3f(self->rest_tail_world,-1.0f,-1.0f,-1.0f);
	vecset3f(self->rest_tail_local,-1.0f,-1.0f,-1.0f);
	self->parent = parent;
	memcpy(self->channels,channels,3*sizeof(int));
	memcpy(self->rot_order,rot_order,3*sizeof(int));
	self->rot_order_str = eul_order_lookup(self->rot_order);
	self->index = index;

	// convenience functions
	self->has_loc = channels[0] != -1 || channels[1] != -1 || channels[2] != -1;
	self->has_rot = channels[3] != -1 || channels[4] != -1 || channels[5] != -1;

	self->children = newStack(struct BVH_Node*);

	// list of 6 length tuples: (lx,ly,lz, rx,ry,rz)
	// even if the channels aren't used they will just be zero
	//
	self->anim_data = newStack(struct anim_record); // [(0, 0, 0, 0, 0, 0)]
	return self;
}

    //def __repr__(self):
    //    return ("BVH name: '%s', rest_loc:(%.3f,%.3f,%.3f), rest_tail:(%.3f,%.3f,%.3f)" %
    //            (self.name,
    //             self.rest_head_world.x, self.rest_head_world.y, self.rest_head_world.z,
    //             self.rest_head_world.x, self.rest_head_world.y, self.rest_head_world.z))


//void sorted_nodes(struct BVH_Nodes *bvh_nodes){
//    bvh_nodes_list = list(bvh_nodes.values())
//    bvh_nodes_list.sort(key=lambda bvh_node: bvh_node.index)
//}
//

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
				token = strtok(line,delims); //OFFSET
				float rest_head_local[3];
				for(int i=0;i<3;i++){
					token = strtok(NULL,delims);
					sscanf(token,"%f",&rest_head_local[i]);
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
        // Account for an end node
        //if file_lines[lineIdx][0].lower() == 'end' and file_lines[lineIdx][1].lower() == 'site':  // There is sometimes a name after 'End Site' but we will ignore it.
        if(!strcasecmp(token,"end")){ // && !strcasecmp(strtok(NULL,delims),"site")){  //End Site
		}
		if(!strcmp(token,"}")){ //}
		}
		if(!strcmp(token,"{")){ //}
		}
        // End of joint hierarchy.
        // start of motion channel float values, starting with:
        //  MOTION
        //  Frames: n
        //  Frame Time: dt
        if(!strcasecmp(token,"motion") ){ //MOTION
            break; //get out of hierarchy loop 
		}
	} //end while lines

	*njoint = mjoint;
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
	for(int i=0;i<*njoint;i++){
		printf("%d\n",i);
		memcpy(&cchan[i],&cjoint[i],sizeof(struct joint_frame_motion));
	}
 
	float * fvalues = malloc( (*channel_count) * (*bvh_frame_count) * sizeof(float));
	*values = fvalues;
	int k = 0;
	char *delims2 = " ,\t\r\n";
	char *str = pos;
	for(int iframe=0;iframe<*bvh_frame_count;iframe++){
        for(int i=0;i<*channel_count;i++){
			token = strtok(str,delims2);
			str = NULL; //so next strtok(NULL,...)
			sscanf(token,"%f",&fvalues[k]);
			k++;
		}
	}

}

//void read_bvh(char *file_path, char *rotate_mode, float global_scale,
//	Stack *bvh_nodes, float *bvh_frame_time, int *bvh_frame_count){
//	char *blob;
//	int len;
//	if( load_file_blob(file_path, &blob, &len) )
//		read_bvh_blob(blob, len, rotate_mode, global_scale, bvh_nodes, bvh_frame_time, bvh_frame_count);
//}

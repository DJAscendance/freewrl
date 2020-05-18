

// license: MIT or similar permissive
/*
BVH includes some skeleton information X3D HAnim doesn't need / can't use:
x length of bones aka OFFSET
* HAnim supplies its own skeleton 
x parenting heirarchy to help accumulate global transform for a limb
* HAnim - we do transforms at each joint, so we need only local joint angles

What X3D needs is HAnim2MotionData.

*/




#include <config.h>
#include <system.h>
#include "Vector.h"
#include "LinearAlgebra.h"

// .H  >> 

void read_bvh(char *file_path, char *rotate_mode, float global_scale,
	Stack *bvh_nodes, float *bvh_frame_time, int *bvh_frame_count);

// << .H


#ifdef _MSC_VER
#define strcasecmp stricmp
#endif //_MSC_VER
#define RADIANS_PER_DEGREE (double)0.0174532925199432957692
#define DEGREES_PER_RADIAN (double)57.2957795130823208768


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
    float *rest_tail_world;  // worldspace rest location for the tail of this node
    float *rest_tail_local;  // worldspace rest location for the tail of this node
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
	*position = &cur[len];
	return *position;
}
struct BVH_Node * init_BVH_Node( char *name, float * rest_head_world, float * rest_head_local, 
	struct BVH_Node *parent, int *channels, int *rot_order, int index){
	struct BVH_Node *self = (struct BVH_Node*)malloc(sizeof(struct BVH_Node));
	memset(self,0,sizeof(struct BVH_Node));

	self->name = name;
	veccopy3f(self->rest_head_world,rest_head_world);
	veccopy3f(self->rest_head_local,rest_head_local);
	self->rest_tail_world = NULL; //,-1.0f,-1.0f,-1.0f);
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

void read_bvh_blob(char *blob, int len, char *rotate_mode, float global_scale,
	Stack *bvh_nodes, float *bvh_frame_time, int *bvh_frame_count)
{
    // File loading stuff
    // Open the file for importing

    // Seperate into a list of lists, each line a list of words.
	char *rv; 
	char line [2048], *pos;
	char *token, *delims;
	pos = blob;
	rv = getline(line,2048,&pos);

    //char * file_lines = fread(file,file.readlines()
    //// Non standard carrage returns?
    //if len(file_lines) == 1:
    //    file_lines = file_lines[0].split('\r')

    // Split by whitespace.
    //file_lines = [ll for ll in [l.split() for l in file_lines] if ll]
	delims = " ,\r\n\\t\"";
	token = strtok(line,delims);
    // Create hierarchy as empties
    //if file_lines[0][0].lower() == 'hierarchy':
	if( strcasecmp(token,"hierarchy")){
		printf("not a BVH file \n");
		return;
	}

    bvh_nodes = NULL;
   // struct BVH_Nodes *bvh_nodes_serial = NULL;
	Stack *bvh_nodes_serial = newStack(struct BVH_Nodes *);
    *bvh_frame_count = 0;
    *bvh_frame_time = 0.0;

	int channelIndex = -1;

    int lineIdx = 0;  // An index for the file.
    //while(lineIdx < len(file_lines) - 1){
	while( getline(line,2048,&pos)){
        //...
		token = strtok(line,delims);
        if(!strcasecmp(token,"root") || !strcasecmp(token,"joint")){
			char *nametokens[4];
			char name[100];
			int len=0;
			memset(nametokens,0,4*sizeof(void*));
			while(nametokens[len] = strtok(NULL,delims)) len++;
            // Join spaces into 1 word with underscores joining it.
			strcpy(name,nametokens[0]);
            // Make sure the names are unique - Object names will match joint names exactly and both will be unique.
			for(int i=1;i<len-1;i++) {
				strcat(name,"_");
				strcat(name,nametokens[i]);
			}
            // MAY NEED TO SUPPORT MULTIPLE ROOTS HERE! Still unsure weather multiple roots are possible?
            //print '%snode: %s, parent: %s' % (len(bvh_nodes_serial) * '  ', name,  bvh_nodes_serial[-1])
			getline(line,2048,&pos); // {
			getline(line,2048,&pos); // OFFSET 8.77824 4.35084 1.2192
            //lineIdx += 2  // Increment to the next line (Offset)
			token = strtok(line,delims);
			float rest_head_local[3];
			for(int i=0;i<3;i++){
				token = strtok(NULL,delims);
				sscanf(token,"%f",&rest_head_local[i]);
			}
            //rest_head_local = Vector((float(file_lines[lineIdx][1]), float(file_lines[lineIdx][2]), float(file_lines[lineIdx][3]))) * global_scale
            //lineIdx += 1  // Increment to the next line (Channels)
			getline(line,2048,&pos); //     CHANNELS 3 Zrotation Xrotation Yrotation
            // newChannel[Xposition, Yposition, Zposition, Xrotation, Yrotation, Zrotation]
            // newChannel references indices to the motiondata,
            // if not assigned then -1 refers to the last value that will be added on loading at a value of zero, this is appended
            // We'll add a zero value onto the end of the MotionDATA so this always refers to a value.
			int my_channel[6];
			for(int i=0;i<6;i++)
				my_channel[i] = -1;
            //my_channel = [-1, -1, -1, -1, -1, -1]
			int my_rot_order[3];
			for(int i=0;i<3;i++)
				my_rot_order[i] = -1;
            //my_rot_order = [None, None, None]
            int rot_count = 0;
			token = strtok(line,delims); //CHANNELS
			token = strtok(NULL,delims); //3
			int channels;
			sscanf(token,"%d",&channels);
            //for channel in file_lines[lineIdx][2:]:
			for(int i=0;i<channels;i++){
				char *channel = strtok(NULL,delims); //Zrotation
                //channel = channel.lower()
                int channelIndex = i+1; // += 1  // So the index points to the right channel
                if(!strcasecmp(channel,"xposition") )
                    my_channel[0] = channelIndex;
                else if(!strcasecmp(channel,"yposition") )
                    my_channel[1] = channelIndex;
                else if(!strcasecmp(channel,"zposition") )
                    my_channel[2] = channelIndex;

                else if(!strcasecmp(channel,"xrotation")){
                    my_channel[3] = channelIndex;
                    my_rot_order[rot_count] = 0;
                    rot_count += 1;
                }else if(!strcasecmp(channel,"yrotation")){
                    my_channel[4] = channelIndex;
                    my_rot_order[rot_count] = 1;
                    rot_count += 1;
                }else if(!strcasecmp(channel,"zrotation")){
                    my_channel[5] = channelIndex;
                    my_rot_order[rot_count] = 2;
                    rot_count += 1;
				}
			}
            //channels = file_lines[lineIdx][2:]

            struct BVH_Node *my_parent = stack_top(struct BVH_Node*,bvh_nodes_serial); //[-1];  // account for none
			float rest_head_world[3];
            // Apply the parents offset accumulatively
            if( my_parent == NULL)
                veccopy3f(rest_head_world,rest_head_local);
            else
                vecadd3f(rest_head_world,my_parent->rest_head_world, rest_head_local);

			struct BVH_Node *bvh_node;
			int index = vectorSize(bvh_nodes_serial) -1;
            bvh_node = init_BVH_Node(name, rest_head_world, rest_head_local, my_parent, my_channel, my_rot_order, index);
			//bvh_nodes[name] = bvh_node
            // If we have another child then we can call ourselves a parent, else
            stack_push(struct BVH_Node*,bvh_nodes_serial,bvh_node);
		}	
        // Account for an end node
        //if file_lines[lineIdx][0].lower() == 'end' and file_lines[lineIdx][1].lower() == 'site':  // There is sometimes a name after 'End Site' but we will ignore it.
        if(!strcasecmp(token,"end") || !strcasecmp(strtok(NULL,delims),"joint")){  //End Site
			getline(line,2048,&pos); // {
			getline(line,2048,&pos); // OFFSET 8.77824 4.35084 1.2192
            //lineIdx += 2  // Increment to the next line (Offset)
			token = strtok(line,delims);
			float rest_tail[3];
			for(int i=0;i<3;i++){
				token = strtok(NULL,delims);
				sscanf(token,"%f",&rest_tail[i]);
			}
			vecscale3f(rest_tail,rest_tail,global_scale);
            //lineIdx += 2  // Increment to the next line (Offset)
            //rest_tail = Vector((float(file_lines[lineIdx][1]), float(file_lines[lineIdx][2]), float(file_lines[lineIdx][3]))) * global_scale

			struct BVH_Node *bvh_node;
			int index = vectorSize(bvh_nodes_serial) -1;
			bvh_node = vector_get(struct BVH_Node*,bvh_nodes_serial,index);
			vecadd3f(bvh_node->rest_tail_world,bvh_node->rest_head_world,rest_tail);
            //bvh_nodes_serial[-1].rest_tail_world = bvh_nodes_serial[-1].rest_head_world + rest_tail
			vecadd3f(bvh_node->rest_tail_local,bvh_node->rest_head_local,rest_tail);
            //bvh_nodes_serial[-1].rest_tail_local = bvh_nodes_serial[-1].rest_head_local + rest_tail

            // Just so we can remove the Parents in a uniform way - End has kids
            // so this is a placeholder
            //bvh_nodes_serial.append(None)
            stack_push(struct BVH_Node*,bvh_nodes_serial,NULL);
		}
        //if len(file_lines[lineIdx]) == 1 and file_lines[lineIdx][0] == '}':  // == ['}']
		if(!strcmp(token,"}")){ //}
            //bvh_nodes_serial.pop()  // Remove the last item
			stack_pop(struct BVH_Nodes*,bvh_nodes_serial);
		}
        // End of the hierarchy. Begin the animation section of the file with
        // the following header.
        //  MOTION
        //  Frames: n
        //  Frame Time: dt
        //if len(file_lines[lineIdx]) == 1 and file_lines[lineIdx][0].lower() == 'motion':
        if(!strcasecmp(token,"motion") ){ //MOTION
            //lineIdx += 1  // Read frame count.
			getline(line,2048,&pos); //Frames:	2752
			token = strtok(line,delims); //frames:
			if(!strcasecmp(token,"frames:")){
				token = strtok(NULL,delims); //2752
				sscanf(token,"%d",bvh_frame_count);
			}
            //if (len(file_lines[lineIdx]) == 2 and
            //    file_lines[lineIdx][0].lower() == 'frames:'):

            //    bvh_frame_count = int(file_lines[lineIdx][1])

            //lineIdx += 1  // Read frame rate. 
			getline(line,2048,&pos); //Frame Time:	0.00833333
			token = strtok(line,delims); //frame
			if(!strcasecmp(token,"frame")){
				token = strtok(line,delims); //time
				if(!strcasecmp(token,"time")){
					token = strtok(line,delims); //0.00833333
					sscanf(token,"%f",bvh_frame_time);
				}
			}
            //if (len(file_lines[lineIdx]) == 3 and
            //    file_lines[lineIdx][0].lower() == 'frame' and
            //    file_lines[lineIdx][1].lower() == 'time:'):

            //    bvh_frame_time = float(file_lines[lineIdx][2])

            //lineIdx += 1  // Set the cursor to the first frame
			//getline(line,2048,&pos); // Set the cursor to the first frame

            break;
		}
		getline(line,2048,&pos); // Set the cursor to the first frame
        //lineIdx += 1
	} //end while lines
 // Remove the None value used for easy parent reference
 // substitute needed

    //del bvh_nodes[None]
    // Dont use anymore
    //del bvh_nodes_serial

    // importing world with any order but nicer to maintain order
    // second life expects it, which isn't to spec.
    //bvh_nodes_list = sorted_nodes(bvh_nodes)

	Stack *bvh_nodes_list = bvh_nodes_serial;
	int nodecount = vectorSize(bvh_nodes_list);
    //while lineIdx < len(file_lines):
	while(getline(line,2048,&pos)){
        //line = file_lines[lineIdx]
        for(int i=0;i<nodecount;i++){ // bvh_node in bvh_nodes_list:
            //for bvh_node in bvh_nodes_serial:
			struct BVH_Node* bvh_node = vector_get(struct BVH_Node*,bvh_nodes_list,i);
            float lx, ly, lz, rx, ry, rz;
			struct anim_record record;
			lx=ly=lz=rx=ry=rz = 0.0f;
            int * channels = bvh_node->channels;
            Stack *anim_data = bvh_node->anim_data;
            //if( channels[0] != -1){
			if(channels[0] != -1){
				sscanf(strtok(line,delims),"%f",&lx);
				lx *= global_scale;
                //lx = global_scale * float(line[channels[0]])
			}
            //if channels[1] != -1:
			if(channels[1] != -1){
				sscanf(strtok(NULL,delims),"%f",&ly);
				ly *= global_scale;
                //ly = global_scale * float(line[channels[1]])
			}
            //if channels[2] != -1:
			if(channels[2] != -1){
				sscanf(strtok(NULL,delims),"%f",&lz);
				lz *= global_scale;
                //lz = global_scale * float(line[channels[2]])
			}

            //if channels[3] != -1 or channels[4] != -1 or channels[5] != -1:
			if(channels[3] != -1 || channels[4] != -1 || channels[5] != -1){
				//rx = radians(float(line[channels[3]]))
				sscanf(strtok(NULL,delims),"%f",&rx);
				rx *= (float)RADIANS_PER_DEGREE;
				//ry = radians(float(line[channels[4]]))
				sscanf(strtok(NULL,delims),"%f",&ry);
				ry *= (float)RADIANS_PER_DEGREE;
				//rz = radians(float(line[channels[5]]))
				sscanf(strtok(NULL,delims),"%f",&rz);
				rz *= (float)RADIANS_PER_DEGREE;
			}
            // Done importing motion data //
			memset(&record,0,sizeof(struct anim_record));
			record.lxyz[0] = lx;
			record.lxyz[1] = ly;
			record.lxyz[2] = lz;
			record.rxyz[0] = rx;
			record.rxyz[1] = ry;
			record.rxyz[2] = rz;
            //anim_data.append((lx, ly, lz, rx, ry, rz))
			stack_push(struct anim_record,anim_data,record);
		}
        //lineIdx += 1
	}

    // Assign children
    //for bvh_node in bvh_nodes_list:
	for(int i=0;i<vectorSize(bvh_nodes_serial);i++){
		struct BVH_Node *bvh_node = vector_get(struct BVH_Node*,bvh_nodes_serial,i);
        struct BVH_Node *bvh_node_parent = bvh_node->parent;
        if(bvh_node_parent)
            stack_push(struct BVH_Node*,bvh_node_parent->children,bvh_node);
	}
    // Now set the tip of each bvh_node
    //for bvh_node in bvh_nodes_list:
	for(int i=0;i<vectorSize(bvh_nodes_serial);i++){
		struct BVH_Node *bvh_node = vector_get(struct BVH_Node*,bvh_nodes_serial,i);

        //if not bvh_node.rest_tail_world:
		if(!bvh_node->rest_tail_world){
            //if len(bvh_node.children) == 0:
			int nchildren = vectorSize(bvh_node->children);
			if(nchildren == 0){
                // could just fail here, but rare BVH files have childless nodes
                //bvh_node.rest_tail_world = Vector(bvh_node.rest_head_world)
				bvh_node->rest_tail_world = bvh_node->rest_head_world;
                bvh_node->rest_tail_local = bvh_node->rest_head_local;
            //elif len(bvh_node.children) == 1:
			}else if(nchildren == 1){
				struct BVH_Node *bvh_child0 = vector_get(struct BVH_Node*,bvh_node->children,0);
                bvh_node->rest_tail_world = bvh_child0->rest_head_world; //[0]->rest_head_world;
                //bvh_node->rest_tail_local = bvh_node.rest_head_local + bvh_node.children[0].rest_head_local

                bvh_node->rest_tail_local = vecadd3f(bvh_node->rest_tail_local_store,bvh_node->rest_head_local,bvh_child0->rest_head_local);
            }else{
                // allow this, see above
                //if not bvh_node.children:
                //	raise Exception("bvh node has no end and no children. bad file")

                // Removed temp for now
                float rest_tail_world[3]; // = Vector((0.0, 0.0, 0.0))
				vecset3f(rest_tail_world,0.0f,0.0f,0.0f);
                float rest_tail_local[3]; // = Vector((0.0, 0.0, 0.0))
				vecset3f(rest_tail_local,0.0f,0.0f,0.0f);
                //for bvh_node_child in bvh_node.children:
				for(int j=0;j<nchildren;j++){
					struct BVH_Node *bvh_child = vector_get(struct BVH_Node*,bvh_node->children,j);
                    //rest_tail_world += bvh_node_child.rest_head_world
					vecadd3f(rest_tail_world,rest_tail_world,bvh_child->rest_head_world);
                    //rest_tail_local += bvh_node_child.rest_head_local
					vecadd3f(rest_tail_local,rest_tail_local,bvh_child->rest_head_local);
				}
				vecscale3f(bvh_node->rest_tail_world,rest_tail_world,1.0f/(float)nchildren);
                //bvh_node.rest_tail_world = rest_tail_world * (1.0 / len(bvh_node.children))
				vecscale3f(bvh_node->rest_tail_local,rest_tail_local,1.0f/(float)nchildren);
                //bvh_node.rest_tail_local = rest_tail_local * (1.0 / len(bvh_node.children))
			}
		}
        // Make sure tail isn't the same location as the head.
		if( vecapprox3f(bvh_node->rest_tail_local,bvh_node->rest_head_world,.001f*global_scale)){
			//if (bvh_node.rest_tail_local - bvh_node.rest_head_local).length <= 0.001 * global_scale:
            printf("\tzero length node found:", bvh_node->name);
            bvh_node->rest_tail_local[1] += global_scale / 10.0f;
            bvh_node->rest_tail_world[1] += global_scale / 10.0f;
		}
	}
	bvh_nodes = bvh_nodes_serial;
    //return bvh_nodes, bvh_frame_time, bvh_frame_count
}

void read_bvh(char *file_path, char *rotate_mode, float global_scale,
	Stack *bvh_nodes, float *bvh_frame_time, int *bvh_frame_count){
	char *blob;
	int len;
	if( load_file_blob(file_path, &blob, &len) )
		read_bvh_blob(blob, len, rotate_mode, global_scale, bvh_nodes, bvh_frame_time, bvh_frame_count);
}

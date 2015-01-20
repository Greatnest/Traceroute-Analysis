#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define StringSize	100 /* Set max file scan string size */
#define ip_size	16 

/* Stores all IP Address into a node */
struct node_struct {
    int id;
    char ipaddr[ip_size];
    struct node_struct *next_node;
    struct prevNode_struct *prevNode;
};
/*Stores all previous nodes of a node*/
struct prevNode_struct {
    int id;
	char ipaddr[ip_size];
    struct prevNode_struct *prevNode;
};

 /*Define aliases for structs */
typedef struct node_struct node;
typedef struct prevNode_struct prevNode;

 /* Create and Add Nodes */
node *create_node(char *ipaddr);
void add_node(node *node_list, node *new_node);

/*Create and Add Previous Nodes*/
prevNode *create_prevNode(char *ipaddr);
void add_prevNode(node *node_ptr, char node_name[], prevNode *new_prevNode);

/* Input file */
void read_file(node *node_list, char *filename);

/* Output file */
void write_file(node *node_list, char *filename);

/* Compare IP addresses*/
int compareIP(char *ip_a, char *ip_b);

/* Convert the IP addresses to long ints for comparison */
long int convertIP(char *ip);

/* Check if IP address is valid */
int validateIP(char *ipAddress);

/* Main Menu */
void mainMenu();

/* Query Menu for IP/ID */
void queryMenu();

/* Get user input */
int getUserChoice(int min, int max);
int input_id();
int distanceInput();
char *input_ip();

/* Menu for passing choice for querying */
void range(node *nd);
void nexthop(node *nd);
void prevhop(node *nd);

/* Processing queries */
void getHopsDistance(node *node_list, int id, int numOfHops);

void getNextHopDistance(node *node_list, int id, int numOfHops);
int getPrevHopDistance(node *node_list, int id, int numOfHops);

void nextHopID(node *node_list, int id);
void nextHopIP(node *node_list, char *ip);

void prevHopID(node *node_list, int id);
void prevHopIP(node *node_list, char *ip);


/* Global variable to keep in check the number of nodes */
int numOfNodes = 1;                          


/* Main Function */
int main(int argc, char *argv[]) {
 
if (argc<3){
	printf("Please provide an input and output file");
	exit(0);
}

int choice;
node *nd= malloc(sizeof(node));     /* Initialize List Head */

read_file(nd, argv[1]);
write_file(nd, argv[2]);

    do {
        mainMenu();                            
        choice = getUserChoice(1,4);

		if(choice==1) {
			prevhop(nd);
		}else if (choice==2) {
			nexthop(nd);
		}else if(choice==3) {
			range(nd);
		} else if (choice==4) {
			printf("Program is exiting!\n");
		} else {	
			printf("Input an integer between 1 and 4\n");
		} 	
		 

    } while(choice != 4);                       

    return 0;
}
/* Prints Main Menu */
void mainMenu() {
    printf("---------------------------------------\n");
    printf("Please select an option from the menu\n");
    printf("1)Report ID of previous hops of network\n");
    printf("2)Identify the next hops of a network node\n");
    printf("3)Print Nodes n-hops away\n");
    printf("4)Exit\n\n");
}

/* User input for main menu*/
int getUserChoice(int min, int max) {
    int choice;
    int validChoice = 0;

    do {                                        /* Scan for user choice until valid */
        printf("Please enter choice => ");

        if(scanf("%d", &choice) == 1) {         /* Check if input is an integer */
            if(((choice >= min) && (choice <= max))) { /* Check if input in choice range */
                validChoice = 1;
            } else {
                printf("Invalid input, please enter an integer between %d-%d \n", min, max);
            }
        } else {
            printf("Invalid input, please enter an integer between %d-%d\n", min, max);
        }
        
    } while(validChoice!=1);

    return choice;
}

/* Read input file for processing of IP Addresses */
void read_file(node *node_list, char *filename) {
    char input[StringSize];
    char ipAddress[StringSize][16];
	
	int i;
    int ip_count = 0;
    node *nd = node_list;

    FILE *file = fopen(filename, "r");          /* Opens file in readonly mode */

    while(fscanf(file, "%s",input) != EOF) {    /* Scan file string by string until the end of the file */
        do {
            fscanf(file, "%s", input);
        }while(strcmp(input, "1") != 0);       /* Scans for start of a traceroute block by looking for first char in the block */

        do {                                     /* Loop until end of ipaddress block */
            fscanf(file, "%s", input);
            
            if(ip_check(input)==1) {             /* Check if scanned string is an IP */
                strcpy(ipAddress[ip_count], input); 
                ip_count++;
            } else if(input[0] == '[') {		 /* Checks for [] in string and converts it to null byte*/
                input[strlen(input) - 1] = '\0'; 
                strcpy(ipAddress[ip_count], input + 1); /* Copies the IP address into the string IP Address*/
                ip_count++;
            }
        }while (strcmp(input, "complete.") != 0); /* Detect end of ipaddr block */
	
        if(strcmp(input, "complete.") == 0) { 
            for(i = 0; i < ip_count; i++) { /* Creates nodes in scanned order */
                add_node(nd, create_node(ipAddress[i]));
            } 
             for(i = 1; i < ip_count; i++) { /* Add in corresponding previous nodes */
                 add_prevNode(nd, ipAddress[i], create_prevNode(ipAddress[i-1]));
             } 
            
            memset(ipAddress, 0, sizeof(ipAddress)); /* Clear traceipaddr storage to scan next ipaddr block */
            ip_count = 0;
        }
    }

    fclose(file);
}

/* Validates IP addresses */
int ip_check (char *ipAddress) {
	int len = strlen(ipAddress);

    if (len < 7 || len > 15) {		/*Check the length of IP Address */
        return 0;
	}
    char tail[16];
    tail[0] = 0;
	int i;
    unsigned int d[4];
    int c;
	
	c = sscanf(ipAddress, "%3u.%3u.%3u.%3u%s", &d[0], &d[1], &d[2], &d[3], tail);	/* formats the IP Address into 4 parts of unsigned integer to ensure all are positive numbers */

    if (c != 4 || tail[0]) {			/* checks that the 4 parts are avaliable or if the first number is 0 */
        return 0;
	}
    
	for (i = 0; i < 4; i++) { 			/* Checks if each part exceeds 255 */
        if (d[i] > 255) {
            return 0;
		}
	}
    
	return 1;
}

/*Converting IP to long int for comparison*/
long int convertIP(char *ip) {
    char part1[3];
    char part2[3];
    char part3[3];
    char part4[3];
    char combined[12];

    sscanf(ip,"%[0-9].%[0-9].%[0-9].%[0-9]", part1, part2, part3, part4); /* Split IP into 4 parts */
    sprintf(combined, "%s%03d%03d%03d", part1, atoi(part2), atoi(part3), atoi(part4));

    return atol(combined);                      /* Convert to long int and return */
}

/* Comparing 2 IP addresses to see which is larger using convertIP function */
int compareIP(char *ip_a, char *ip_b) {
    if(convertIP(ip_a) > convertIP(ip_b)) {
        return 1;
    } else {
        return 0;
    }
}

/* Takes in the IP address and create a node using it */
node *create_node(char *ipaddr) {
    node *new_node = malloc(sizeof(node)); /* Allocate memory for node on the heap */

    strcpy(new_node->ipaddr, ipaddr);        /* Assigns IP address to the node*/
    new_node->prevNode = 0;
    new_node->next_node = 0;
    new_node->id = numOfNodes;                /* assign node unique ID */

    return new_node;
}


/* Inserting the node into list by ascending value of IP Address */
void add_node(node *node_list, node *new_node) {
    node *node_check = node_list; /**/            

    while(node_check->next_node != 0) {	/*check if end of list or not*/
        node_check = node_check->next_node;

        if(strcmp(node_check->ipaddr, new_node->ipaddr) == 0) {
            return;                             /* exit function if node exists */
        }
    }
    
    node *node = node_list;

    if(node->next_node == 0) {                  /* handle empty list */
        node_list->next_node = new_node;
        numOfNodes++;
    } else {                                    /* insertion sort */
        if(compareIP(node->ipaddr, new_node->ipaddr)==1) { 
            node_list = new_node;
            new_node->next_node = node;
            numOfNodes++;
        } else {
            while(node->next_node != 0 && compareIP(new_node->ipaddr, node->next_node->ipaddr)==1) {
                node = node->next_node;
            }
            new_node->next_node = node->next_node;
            node->next_node = new_node;
            numOfNodes++;                               /* update number of nodes */
            }
    }
}

/* create a node in order to store the previous nodes */
prevNode *create_prevNode(char ipaddr[]) {
    prevNode *new_prevNode = malloc(sizeof(prevNode));
   
    strcpy(new_prevNode->ipaddr, ipaddr);
    new_prevNode->prevNode = 0;
    return new_prevNode;
}


/* Inserts a prevNode into the list */
void add_prevNode(node *node_ptr, char node_name[], prevNode *new_prevNode) {	  
	node *node_search = node_ptr->next_node;
	node *node = node_ptr->next_node;
 
    while(node_search->next_node != 0) {    
        if(strcmp(node_search->ipaddr, new_prevNode->ipaddr) == 0) {
            break;
        }
        node_search = node_search->next_node;
    }

    new_prevNode->id = node_search->id;           
	
    while(strcmp(node_name, node->ipaddr) != 0) { 
        node = node->next_node;
    }

    if(node->prevNode == 0) {                    
        node->prevNode = new_prevNode;
    } else {
        prevNode *pN = node->prevNode;

        while(pN->prevNode != 0) {            
            if(strcmp(pN->ipaddr, new_prevNode->ipaddr) == 0) {
                return;                       
            }
            pN = pN->prevNode;
        }

        if(strcmp(pN->ipaddr, new_prevNode->ipaddr) == 0) {
            return;                             /* Exits if last prevNode is identical */
        }

        pN->prevNode = new_prevNode;               /* Insert new prevNode */
    }
}

 
/*Writing output to file*/
void write_file(node *node_list, char *filename) {
    FILE *file = fopen(filename, "w");           /* Open file input file in write mode */

    node *node = node_list->next_node;

    while (node != NULL) {                        /* Traverse list and print output to output file */
                                                   
        char part1[3];
        char part2[3];
        char part3[3];
        char part4[3];

        sscanf(node->ipaddr, "%[0-9].%[0-9].%[0-9].%[0-9]",part1, part2, part3, part4);  /* Split IP into 4 parts to format */ 
		fprintf(file, "%2d: %3s.%3s.%3s.%3s <-- ", node->id, part1, part2, part3, part4);
		printf("%2d: %3s.%3s.%3s.%3s <-- ",  node->id, part1, part2, part3, part4);

        prevNode *pN = node->prevNode;

        while(pN != NULL) {
            if(pN->prevNode == NULL) {
                fprintf(file, "%2d", pN->id);  /*check if have more than 1 node pointing to another node*/
                printf("%2d", pN->id);
            } else {
                fprintf(file, "%2d, ", pN->id);
                printf("%2d, ", pN->id);
                }
                pN = pN->prevNode;
            }

        node = node->next_node;
        fprintf(file, "\n");
        printf("\n");
    }
    fclose(file);
}

			
/* Query Menu */
void queryMenu() {
    printf("\nSearch by:\n");
    printf("1)ID\n");
    printf("2)IP address\n");
}

/* Input for ID*/
int input_id() {
    int id; 
    printf("Please enter ID => ");
    scanf("%d", &id);
    return id;
}

/* Input for IP Address */
char *input_ip() {
    char *ip = malloc(sizeof(char[StringSize])); 
    printf("Please enter IP => ");
    scanf("%s", ip);
    return ip;
}

/* Checks user input for ID/IP for previous hops */
void prevhop(node *nd) {
    queryMenu();
    int choice = getUserChoice(1,2);

	if(choice==1) {
	prevHopID(nd, input_id()); 
	}else {
	prevHopIP(nd, input_ip());
	}
}

/* Checks user input for ID/IP for next hops*/
void nexthop(node *nd) {
    queryMenu();
    int choice = getUserChoice(1,2);

	if(choice==1) {
	nextHopID(nd, input_id()); 
	}else {
	nextHopIP(nd, input_ip());
	}
	
}

/* Get next nodes using ID as input*/
void nextHopID(node *node_list, int id) {
    node *nd = node_list->next_node;

    while(nd != NULL) {                          /* Traverse all nodes */
        prevNode *pN = nd->prevNode;
        while(pN != NULL) {                     /* Traverse all previous nodes */
            if(pN->id == id) {                  /* Prints ID and IP of corresponding  node if previous node matches user input */
                printf("ID: %02d IP: %s\n", nd->id, nd->ipaddr);
            }
            pN = pN->prevNode;
        }
        nd = nd->next_node;
    }
}

/* Get next nodes using IP as input*/
void nextHopIP(node *node_list, char *ip) {
    node *nd = node_list->next_node;

    while(nd != NULL) {                         /* Traverse all nodes */
        prevNode *pN = nd->prevNode;
        while(pN != NULL) {                     /* Traverse all previous nodes */
            if(strcmp(pN->ipaddr, ip) == 0) {     /* Prints ID and IP of corresponding node if previous node matches user input*/     
                printf("ID: %02d IP: %s\n", nd->id, nd->ipaddr);
            }
            pN = pN->prevNode;
        }
        nd = nd->next_node;
    }
}
/* Get previous nodes using ID as input*/
void prevHopID(node *node_list, int id) {
    node *nd = node_list->next_node;

    while(nd != NULL) {                         /* Traverse all nodes until user inputted ID matches node ID */
        if(nd->id == id) {
            prevNode *pN = nd->prevNode;
            while(pN != NULL) {                 /* Prints all previous nodes of associated node */
                printf("ID: %02d IP: %s\n", pN->id, pN->ipaddr);
                pN = pN->prevNode;
            }
            return;
        }
        nd = nd->next_node;
    }
}


/* Get previous nodes using IP as input*/
void prevHopIP(node *node_list, char *ip) {
    node *nd = node_list->next_node;

    while(nd != NULL) {                         /* Traverse all nodes until user query matches node IP */
        if(strcmp(nd->ipaddr, ip) == 0) {
            prevNode *pN = nd->prevNode;
            while(pN != NULL) {                 /* Prints all previous nodes of associated node */
                printf("ID: %02d IP: %s\n", pN->id, pN->ipaddr);
                pN = pN->prevNode;
            }
            return;
        }
        nd = nd->next_node;
    }
}
/* Input for hop distance */
int distanceInput() {
    int distanceInput = 0;
    printf("Please enter hop distance => ");
    scanf("\n%d", &distanceInput);
    return distanceInput;
}

/* Checks user input for hop distance*/
void range(node *nd) {
    getHopsDistance(nd, input_id(), distanceInput());
}

/* Function to get next or previous hops using distanceInput */
void getHopsDistance(node *node_list, int id, int numOfHops) {
    getNextHopDistance(node_list, id, numOfHops);
    getPrevHopDistance(node_list, id, numOfHops);
}


/* function to get next hops using distanceInput */
void getNextHopDistance(node *node_list, int id, int numOfHops) {   
   if(numOfHops == 0) {                       			/* return if max range reached */
        return;
    } else {
        node *nd = node_list->next_node;
        while(nd != NULL) {             		        /* Traverse all nodes */
            prevNode *pN = nd->prevNode;
            while(pN != NULL) {        		   			/* Traverse all prevNodes until matches user query */
                if(pN->id == id) {
                     if(numOfHops == 1) {      			/* Prints node info if at specified distance */
                         printf("ID: %d IP: %s\n", nd->id, nd->ipaddr);
                     } else {                   		/* else search next hop by calling the function again */
                         getNextHopDistance(node_list, nd->id, numOfHops - 1);
                     }	 
                }
                pN = pN->prevNode;
            }
            nd = nd->next_node;
        }  
    }
}
/* function to get previous hops using distanceInput */
int getPrevHopDistance(node *node_list, int id, int numOfHops) {
    if(numOfHops == 0) {                       			/* Return if number of hops reached */
        return;
    } else {
        node *nd = node_list->next_node;
        while(nd != NULL) {                     		/* Traverse all nodes until user query matched */
            if(nd->id == id) {
                prevNode *pN = nd->prevNode;
                while(pN != NULL) {             		/* Traverse all previous nodes */
                    if(numOfHops == 1) {       			/* Print node info if at specified distance */
                        printf("ID: %d IP: %s\n", pN->id, pN->ipaddr);
                    }else 	{                          /* else continue searching previous hops  by calling the function again*/
                    getPrevHopDistance(node_list, pN->id, numOfHops - 1);
                    return;
					}pN = pN->prevNode;
                }
                
            }
            nd = nd->next_node;
        }
    }
}

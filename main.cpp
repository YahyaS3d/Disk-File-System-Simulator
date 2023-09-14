//Yahya Saad
//ID: 322944869


//------some included libraries------
#include <iostream>
#include <vector>
#include <map>
#include <cassert>
#include <cstring>
#include <cmath>
//#include <sys/types.h>
#include <unistd.h>
//#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
// #define SYS_CALL
#define DISK_SIZE 512
#define DISK_SIM_FILE "DISK_SIM_FILE.txt"
//------helper functions------
// Function to convert a decimal number 'n' to its binary representation
// and store it in the character 'result'.
void decimalToBinary(int n, char &result) {
    // Array to store the binary representation (up to 8 bits)
    int binaryArray[8];

    // Initialize the index for the binaryArray
    int index = 0;

    // Convert decimal to binary
    while (n > 0) {
        // Calculate the remainder when dividing by 2 (0 or 1)
        binaryArray[index] = n % 2;

        // Divide the number by 2 to move to the next bit
        n = n / 2;

        // Increment the index
        index++;
    }

    // Convert the binaryArray to a character 'result'
    for (int j = index - 1; j >= 0; j--) {
        if (binaryArray[j] == 1) {
            // Set the j-th bit of 'result' to 1 using bitwise OR
            result = result | (1u << j);
        }
    }
}

// Function to convert a decimal number 'n' to its binary representation
// and store it in the unsigned char 'c'.
void finalDec(int n, unsigned char &c) {
    // Array to store the binary representation (up to 8 bits)
    int binaryArray[8];

    // Initialize the index for the binaryArray
    int index = 0;

    // Convert decimal to binary using the decimalToBinary function
    char result = 0;
    decimalToBinary(n, result);

    // Convert the binaryArray to an unsigned char 'c'
    for (int j = 7; j >= 0; j--) {
        if ((result >> j) & 1) {
            // Set the j-th bit of 'c' to 1 using bitwise OR
            c = c | (1u << j);
        }
    }
}


//fs Inode class

class fsInode {
    int fileSize;
    int block_in_use;

    int *directBlocks; //pointer for 3 or more direct blocks

    int doubleInDirect;
    int singleInDirect;
    int num_of_direct_blocks;// in our case it's only 1-2-3

public:
    fsInode(int _block_size, int _num_of_direct_blocks) {
        fileSize = 0;
        block_in_use = 0;
        block_size = _block_size;
        num_of_direct_blocks = 2;
        directBlocks = new int[num_of_direct_blocks];
        assert(directBlocks);
        for (int i = 0 ; i < num_of_direct_blocks; i++) {
            directBlocks[i] = -1;
        }
        singleInDirect = -1;
        doubleInDirect = -1;

    }

    ~fsInode() {
        delete [] directBlocks;
        block_size=0;
    }

    //------fsInode private function------


    int getUsedBlocks() // Returns the number of used data blocks
    {
        return block_in_use;
    }

    void updateUsedBlocks(int d) // Sets the change to the amount of used blocks
    {
        block_in_use += d;
    }
    // Getter function for fileSize
    int GetFileSize() const {
        return fileSize;
    }
    int* getDirect()
    {
        return directBlocks;
    }
    // Getter for singleInDirect
    int getSingleIndirect()
    {
        return singleInDirect;
    }
    // Setter for singleInDirect
    void setSingleIndirect(int i)
    {
        singleInDirect = i;
    }
     // Getter for doubleInDirect
    int getDoubleIndirect()
    {
        return doubleInDirect;
    }
     // Setter for doubleInDirect
    void setDoubleIndirect(int i)
    {
        doubleInDirect = i;
    }

    // Setter function for fileSize
    void SetFileSize(int size) {
        fileSize = size;
    }

    int block_size;
};
//------------------------------

//File Descriptor class

class FileDescriptor {
    pair<string, fsInode*> file; // pair struct!
    bool state; //flag to check file state
    bool inUse; //flag to check the usage
    

public:
    //constructor
    FileDescriptor(string FileName, fsInode* fsi) {
        file.first = FileName;
        file.second = fsi;
        inUse = true;
        state=false;
    }
    // Destructor for FileDescriptor class
    ~FileDescriptor()
    {
        // Free any dynamically allocated memory
        if (file.second != nullptr)
        {
            delete file.second; // Delete the associated fsInode
            file.second = nullptr;
        }

        // No need to free file.first as it's a string (automatic memory management)
    }
    bool isEmpty() //second file is 
    {return file.second == nullptr;}
    void killString()
    {
        if(&file.first !=nullptr)
            free(&file.first);
    }
    int GetFileSize() {
        // Check if the file descriptor is empty (no associated inode).
        if (isEmpty() || file.second == nullptr) {
            return -1; // Return -1 to indicate an error (no file or inode associated).
        }

        // Calculate the file size based on the block size stored in the fsInode.
        int blockSize = file.second->block_size; // Use the 'block_size' member directly.
        int blocksInUse = file.second->getUsedBlocks(); // Use the existing method in fsInode.
        int fileSize = blockSize * blocksInUse;

        return fileSize;
    }

    void killInode()
    {
        if(&file.second !=nullptr)
            delete file.second;
    }

    //setters and getters
    string getFileName(){
        return file.first;
    }
    fsInode* getInode() {
        return file.second;}
    bool isInUse() {return (inUse);
    }
    void setInUse(bool _inUse) {
        inUse = _inUse;
    }
    void setstate() {
        state=true;inUse=false;
    }
    bool getstate() {
        return state;
    }
};



//fsDisk class
// This class appears to be part of a file system implementation, managing a simulated disk and various file system-related data structures.

class fsDisk {
    FILE *sim_disk_fd; // Pointer to the simulated disk file.

    bool is_formated; // Flag indicating whether the disk is formatted.

    int BitVectorSize; // Size of the bit vector used for block allocation.
    int *BitVector; // Bit vector for tracking allocated and free blocks.
    int *UsedBitVector; // Bit vector for tracking used blocks.
    int *indBitVector; // Bit vector for tracking indirect blocks.

    // Unix directories are lists of association structures,
    // each of which contains one filename and one inode number.
    map<string, fsInode*>  MainDir; // Main directory structure mapping filenames to inodes.

    // OpenFileDescriptors -- when you open a file,
    // the operating system creates an entry to represent that file.
    // This entry number is the file descriptor.
    vector< FileDescriptor > OpenFileDescriptors; // Vector to manage open file descriptors.

    int direct_enteris; // Number of direct entries in an inode.
    int block_size; // Size of data blocks on the disk.

public:


    // Constructor for initializing the fsDisk all object
    fsDisk()
    {
        sim_disk_fd = fopen( DISK_SIM_FILE , "wr+" ); // Open the simulated disk file for read and write operations.
        assert(sim_disk_fd);
        for (int i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fwrite( "\0" ,  1 , 1, sim_disk_fd );
            assert(ret_val == 1);
        }
        fflush(sim_disk_fd);
        is_formated=false; // Initialize the formatted flag to false.
    }

    //cleaning up resources
    ~fsDisk()
    {
        fclose(sim_disk_fd); // Close the simulated disk file.
        remove(DISK_SIM_FILE); // Delete the disk file if it's formatted.

        if(is_formated)
        {
            delete [] BitVector; // Release memory for the bit vector.
            delete [] UsedBitVector; // Release memory for the used bit vector.
            delete [] indBitVector; // Release memory for the indirect bit vector.

            // Release resources associated with open file descriptors.
            while(!OpenFileDescriptors.empty())
            {
                int s = OpenFileDescriptors.size()-1;
                if(!OpenFileDescriptors[s].getstate())
                    delete OpenFileDescriptors[s].getInode();
                OpenFileDescriptors.pop_back();
            }
        }
    }

    //-----------some helper function for fs inode class ---------
    bool Disk_format() //will return whether the fsDisk is 'formatted'
    {

        return is_formated;

    }
    int writeToOffset(int offset, char c) {
        // Open the disk simulation file in read-write mode.
        int fd = open(DISK_SIM_FILE, O_RDWR, 0);

        // Seek to the specified offset within the file.
        lseek(fd, offset, SEEK_SET);

        // Write the character 'c' at the specified offset.
        write(fd, &c, 1);

        // Close the file after writing.
        close(fd);

        // Return 0 to indicate successful write operation.
        return 0;
    }

    int freedBlock() //will tell us how many blocks are still available on the disk
    {
        int i=this->BitVectorSize;
        for(int j=0;j<BitVectorSize;j++)
            if(BitVector[j] == 1)
                i--;
        return i;
    }
    char* readBlock(int blockNumber) {
        // Allocate memory for the block data.
        char *blockData = new char[block_size];

        // Open the disk simulation file in read-only mode.
        int fd = open(DISK_SIM_FILE, O_RDONLY, 0);

        // Seek to the specified block within the file.
        lseek(fd, blockNumber * block_size, SEEK_SET);

        // Read the block data from the file.
        read(fd, blockData, block_size);

        // Close the file after reading.
        close(fd);

        // Return the read block data.
        return blockData;
    }
    int getFreeBlock() {
        int i = 0;

        // Iterate through the BitVector to find a free block.
        for (; i < BitVectorSize; i++) {
            if (BitVector[i] == 0) {
                break; // Found a free block, exit the loop.
            }
        }

        // If no free block is found, return -1 to indicate no available blocks.
        if (i == BitVectorSize) {
            return -1;
        }

        if (UsedBitVector[i] == 1) {
            // If the block is pre-used, clean the cells in it by setting them to '\0'.
            for (int j = 0; j < block_size; j++) {
                writeToOffset(i * block_size + j, '\0');
            }
        }

        // Return the index of the free block.
        return i;
    }
    //lists all the files currently in our MainDir
    void listAll() {
        //index iterator
        int i = 0;
        //check every index from the beginning to the end
        for ( auto it = begin (OpenFileDescriptors); it != end (OpenFileDescriptors); ++it) {
            cout << "index: " << i << ": FileName: " << it->getFileName() <<  " , isInUse: "
                 << it->isInUse() << " file Size: " << it->GetFileSize() << endl;
            i++;
        }
        char bufy;
        cout << "Disk content: '" ;
        for (i=0; i < DISK_SIZE ; i++) {
            int ret_val = fseek ( sim_disk_fd , i , SEEK_SET );
            ret_val = fread(  &bufy , 1 , 1, sim_disk_fd );
            cout << bufy;
        }
        cout << "'" << endl;


    }


// Function to create a new file and add its pointer to MainDir and OpenFileDescriptors
    int CreateFile(string fileName) {
        // Check if the file already exists in MainDir
        if (MainDir.count(fileName) > 0) {
            cerr << "ERROR: filename has been chosen before, please select another." << endl;
            return -1;
        }

        // Create a new fsInode for the file
        fsInode* new_file = new fsInode(this->block_size, this->direct_enteris); // Create a new fsInode

        // Add the new fsInode to MainDir
        MainDir.emplace(fileName, new_file);

        // Create a new FileDescriptor for the file
        FileDescriptor tmp_file = FileDescriptor(fileName, new_file); // Create a new FileDescriptor

        // Add the FileDescriptor to OpenFileDescriptors
        OpenFileDescriptors.push_back(tmp_file);

        // Return the index of the newly created file in OpenFileDescriptors
        // (the index is one less than the size of OpenFileDescriptors)
        return this->OpenFileDescriptors.size() - 1;
    }

    // Function to format the file system disk with specified parameters
    void fsFormat(int blockSize = 4, int directEntries = 3) {
        // Check if the blockSize is larger than the disk size
        if (blockSize > DISK_SIZE) {
            cerr << "ERROR: Block size exceeds the available disk space (" << blockSize << " bytes > " << DISK_SIZE << " bytes)." << endl;
            return;
        }else if (directEntries < 0) {
            cerr << "ERROR: Direct entries quantity cannot be negative." << endl;
            return;
        }else if (blockSize <= 0) {
            cerr << "ERROR: Block size must be greater than zero and include at least one direct block." << endl;
            return;
        }

        // Set the file system parameters
        this->direct_enteris = directEntries;
        this->block_size = blockSize;
        this->is_formated = true; // set the flag
        this->BitVectorSize = DISK_SIZE / block_size;

        // Initialize BitVector, UsedBitVector, and indBitVector arrays
        this->BitVector = new int[BitVectorSize];
        this->UsedBitVector = new int[BitVectorSize];
        this->indBitVector = new int[BitVectorSize];
        for (int i = 0; i < BitVectorSize; i++) {
            BitVector[i] = 0;
            UsedBitVector[i] = 0;
            indBitVector[i] = 0;
        }

        // Calculate the number of blocks on the disk
        int blocks_number = (DISK_SIZE) / (block_size * 2);

        cout << "FORMAT DISK: Number of blocks created: " << blocks_number << endl;

        // Warn if the disk size is suboptimal
        if (blocks_number == 1)
            cerr << "WARNING: The disk size may be suboptimal for the selected block size (<" << (DISK_SIZE) / 2 << " bytes)." << endl;
    }

    // Function to open a file. The 'inUse' flag is changed to true to mark it as used.
    int OpenFile(string FileName) {
        bool fileFound = false; // Flag to check if the file has been found
        int i = 0;
        // Iterate through the OpenFileDescriptors to find the file
        for (auto it = begin(OpenFileDescriptors); it != end(OpenFileDescriptors); ++it) {
            if (it->getFileName() == FileName) {
                fileFound = true;
                break;
            }
            i++;
        }

        // If the file was not found, return an error
        if (!fileFound) {
            cerr << "ERROR: The specified filename does not exist." << endl;
            return -1;
        } else if (OpenFileDescriptors[i].getstate()) {
            // If the file is already open, return an error
            cerr << "ERROR: The file is already open." << endl;
            return -1;
        }

        // Set the 'inUse' flag to true to mark the file as used
        OpenFileDescriptors[i].setInUse(true);
        return i; // Return the index of the opened file
    }



// Function to close a file associated with the given file descriptor 'fd'.
    string CloseFile(int fd) {
        // Check if the file descriptor 'fd' is out of bounds or if it represents an empty file descriptor.
        if (fd >= OpenFileDescriptors.size() || OpenFileDescriptors[fd].isEmpty()) {
            cerr << "ERROR: Invalid file descriptor." << endl;
            return "-1"; // Return an error code ("-1") to indicate failure.
        } else if (OpenFileDescriptors[fd].getstate()) {
            // If the file is already closed, return an empty string to indicate success.
            return "";
        }

        // Set the 'inUse' flag too false to mark the file as closed.
        OpenFileDescriptors[fd].setInUse(false);

        // Return the name of the closed file.
        return OpenFileDescriptors[fd].getFileName();
    }


// Function to write data from the buffer 'buf' to a file with the specified file descriptor 'fd'.
    int WriteToFile(int fd, char *buf, int len) {
        fsInode *toUse = OpenFileDescriptors[fd].getInode();

        //OpenFileDescriptors cases --> check all objects
        {
            int avgSize = ((this->direct_enteris + this->block_size) * this->block_size);
            //case(1) --> chk allowed length
            if (avgSize < len) {
                // Print an error message if the string length exceeds the allowed maximum.
                cerr << "ERROR: String length exceeds the allowed maximum (" << avgSize << " bytes)." << endl;
                return -1;
                //case(2) --> chk fd size and if it's under the roles
            } else if (fd >= this->OpenFileDescriptors.size() || this->OpenFileDescriptors[fd].isEmpty()) {
                // Print an error message for an invalid file descriptor.
                cerr << "ERROR: Invalid file descriptor." << endl;
                return -1;
                //case(3) --> fd state
            } else if (OpenFileDescriptors[fd].getstate()) {
                // Print an error message if the file is deleted.
                cerr << "ERROR: File is deleted." << endl;
                return -1;
                //case(4) --> fd usage flag
            } else if (!OpenFileDescriptors[fd].isInUse()) {
                // Print an error message if the file is not open.
                cerr << "ERROR: File is not open. Please open the file first." << endl;
                return -1;
            }

            int ava_usage = avgSize - toUse->getUsedBlocks();
               //case(5) --> the block is available to use
            if (ava_usage < len) {
                // Print an error message if there is not enough room available for writing.
                cerr << "ERROR: Not enough room available for writing." << endl;
                return -1;
            }
        }

        int char_usage = 0; // IndA can be used to indicate how many characters have been added
        int *tmp_dir = toUse->getDirect();

        // Add to direct entries if possible
        for (int i = 0; i < this->direct_enteris && char_usage < len && toUse->getUsedBlocks() < this->direct_enteris * this->block_size; i++) {
            int j = tmp_dir[i];
            if (j == -1) {
                j = getFreeBlock();
                if (j == -1) {
                    // Print an error message if there are no available blocks for allocation.
                    cerr << "ERROR: No available blocks for allocation. Please delete some blocks first." << endl;
                    return -1;
                }
                tmp_dir[i] = j;
                BitVector[j] = 1;
                UsedBitVector[j] = 1;
            }
            char *char_usage_tmp = readBlock(j);
            for (int k = 0; k < this->block_size && char_usage < len; k++) {
                if (char_usage_tmp[k] == '\0') {
                    this->writeToOffset(j * this->block_size + k, buf[char_usage]);
                    char_usage++;
                }
            }
            delete[] char_usage_tmp; //malloc the memory
        }

        // Add to indirect entries if needed and possible
        if (char_usage < len) {
            if (toUse->getSingleIndirect() == -1) {
                int i = getFreeBlock();
                if (i == -1) {
                    // Print an error message if there is no space available for full string storage.
                    cerr << "ERROR: No space available for full string storage." << endl;
                    return -1;
                }
                toUse->setSingleIndirect(i);
                BitVector[i] = 1;
                UsedBitVector[i] = 1;
                indBitVector[i] = 1;
            }

            int calcBlock = (this->block_size * toUse->getSingleIndirect());
            char *s_indirect = readBlock(toUse->getSingleIndirect());
            for (int i = 0; i < this->block_size && char_usage < len; i++) {
                char d_indirect = s_indirect[i]; //double indirect
                int num = (int)d_indirect;
                if (d_indirect == 0) {
                    int j = getFreeBlock();
                    if (j == -1) {
                        // Print an error message if there are no available blocks for allocation.
                        cerr << "ERROR: No available blocks for allocation. Please delete some blocks first." << endl;
                        return -1;
                    }
                    BitVector[j] = 1;
                    UsedBitVector[j] = 1;
                    unsigned char c = 0;
                    finalDec(j, c);
                    writeToOffset(calcBlock + i, c);
                    num = j;
                }
                char *block_arr = readBlock(num);
                for (int k = 0; k < this->block_size && char_usage < len; k++) {
                    if (block_arr[k] == '\0') {
                        writeToOffset(num * this->block_size + k, buf[char_usage]);
                        char_usage++;
                    }
                }
                delete[] block_arr;
            }
            delete[] s_indirect;
        }

        // Check if all characters in the string have been assigned
        if (char_usage != len) {
            // Print a warning if there is not enough available space for the entire string.
            cerr << "WARNING: Not enough available space for the entire string." << endl;
        }
        return 0;
    }



    //delete the file from the mainDir and other stuff
    int DelFile( string FileName )
    {
        auto it=begin (OpenFileDescriptors);
        int j=0;
        for ( ; it != end (OpenFileDescriptors); ++it)
        {
            if(it->getFileName() == FileName)
                break;
            j++;
        }
        if(it->getstate())
        {
            cerr << "ERROR: file already deleted." << endl;
            return -1;
        }

        if(MainDir.find(FileName)==MainDir.end()) //if you cannot find it in the 'directory'
        {
            cerr << "ERROR: no file found with this name." << endl;
            return -1;
        }
        fsInode *f = MainDir.at(FileName);
        MainDir.erase(FileName);

        for(int i=0;i<direct_enteris;i++)
        {
            if(f->getDirect()[i]!=-1)
            {
                this->BitVector[f->getDirect()[i]]=0;
            }
        }
        if(f->getSingleIndirect() != -1) //if any indirect blocks exist within the file!
        {
            int s_indirect = f->getSingleIndirect();//where the indirect block starts!
            char* tmp = new char[block_size];

            int value = fseek ( sim_disk_fd , (s_indirect*block_size) , SEEK_SET );
            value = fread(  tmp , 1 , block_size, sim_disk_fd);
            for(int i=0;i<block_size;i++)
            {
                int x=(int)tmp[i];
                if(x!=0)
                    BitVector[x]=0;
            }
            BitVector[s_indirect]=0;
            indBitVector[s_indirect]=0;
        }
        delete f; //malloc memory space
        it->setstate();
        return j;
    }
    int GetFileSize(int fd) {
        // Check if the file descriptor is valid.
        if (fd < 0 || fd >= OpenFileDescriptors.size()) {
            cerr << "ERROR: Invalid file descriptor." << endl;
            return -1;
        }

        // Get the file size using the associated inode.
        fsInode *inode = OpenFileDescriptors[fd].getInode();
        if (inode == nullptr) {
            cerr << "ERROR: File descriptor is not associated with an inode." << endl;
            return -1;
        }

        return inode->GetFileSize();
    }


    //copy file content
    int CopyFile(string srcFileName, string destFileName) {
        // Check if the source file exists.
        fsInode* srcInode = nullptr;
        for (const auto& entry : MainDir) {
            if (entry.first == srcFileName) {
                srcInode = entry.second;
                break;
            }
        }

        if (srcInode == nullptr) {
            cerr << "ERROR: Source file '" << srcFileName << "' not found." << endl;
            return -1;
        }

        // Check if the destination file already exists.
        for (const auto& entry : MainDir) {
            if (entry.first == destFileName) {
                cerr << "ERROR: Destination file '" << destFileName << "' already exists." << endl;
                return -1;
            }
        }

        // Create a new file with the destination name.
        fsInode* destInode = new fsInode(srcInode->block_size, 2); // Assuming 2 direct blocks.
        MainDir[destFileName] = destInode;

        // Copy the file size from the source inode to the destination inode.
        destInode->SetFileSize(srcInode->GetFileSize());

        return 0;
    }




    //move the file function!
    int MoveFile(string srcFileName, string destFileName) {
        // Check if the source file exists.
        fsInode* srcInode = nullptr;
        auto srcIter = MainDir.find(srcFileName);
        if (srcIter != MainDir.end()) {
            srcInode = srcIter->second;
        } else {
            cerr << "ERROR: Source file '" << srcFileName << "' not found." << endl;
            return -1;
        }

        // Check if the destination file already exists.
        if (MainDir.find(destFileName) != MainDir.end()) {
            cerr << "ERROR: Destination file '" << destFileName << "' already exists." << endl;
            return -1;
        }

        // Create a new file entry with the destination name.
        MainDir[destFileName] = srcInode;

        // Remove the source file entry from the directory.
        MainDir.erase(srcIter);

        return 0;
    }



    //rename the existing file
    int RenameFile(string oldFileName, string newFileName) {
        // Check if the old file exists.
        fsInode* oldInode = nullptr;
        for (const auto& entry : MainDir) {
            if (entry.first == oldFileName) {
                oldInode = entry.second;
                break;
            }
        }

        if (oldInode == nullptr) {
            cerr << "ERROR: Source file '" << oldFileName << "' not found." << endl;
            return -1;
        }

        // Check if the new file name is already in use.
        for (const auto& entry : MainDir) {
            if (entry.first == newFileName) {
                cerr << "ERROR: Destination file name '" << newFileName << "' is already in use." << endl;
                return -1;
            }
        }

        // Remove the old file entry from the directory.
        MainDir.erase(oldFileName);

        // Add the old file with the new name to the directory.
        MainDir[newFileName] = oldInode;

        return 0;
    }

    //read from file content
    int ReadFromFile(int fd, char *buf, int len) {

        // Check for errors!
        {
            if (fd >= this->OpenFileDescriptors.size() || this->OpenFileDescriptors[fd].isEmpty()) {
                cerr << "ERROR: there is no fd in spot " << fd << "." << endl;
                return -1;
            }else if (OpenFileDescriptors[fd].getstate()) {
                cerr << "ERROR: file is deleted." << endl;
                return -1;
            }else if (!OpenFileDescriptors[fd].isInUse()) {
                cerr << "ERROR: file is not open, please open file first!" << endl;
                return -1;
            }
        }

        int first = 0; //init the first indirect block
        fsInode *usedN = this->OpenFileDescriptors[fd].getInode();

        // Take as much as needed
        {
            for (int i = 0; i < this->direct_enteris && i < len; i++) {
                int m = usedN->getDirect()[i];
                if (m == -1) { //possible from direct blocks
                    continue;
                }
                char *arr = readBlock(m);
                for (int j = 0; j < block_size && first < len; j++) {
                    if (arr[j] != '\0') {
                        buf[first] = arr[j];
                        first++;
                    }
                }
                delete[] arr; //malloc the memory
            }
        }

       //If the user needs more data than what can be directly stored in the initial set of blocks allocated to the file
       //there are characters stored on the physical disk corresponding to the file's content
       //retrieve and append those characters to the output string.
        if (len > direct_enteris && first < len) {
            int singleIndirectBlockStart = usedN->getSingleIndirect(); // Where the single indirect block starts!
            char *directBlockData = new char[block_size];
            char *indirectBlockData = readBlock(singleIndirectBlockStart);
            for (int i = 0; i < block_size && first < len; i++) {
                char blockIndexChar = indirectBlockData[i];
                // check block (i)
                int blockIndex = (int)blockIndexChar;
                if (blockIndex == 0 || blockIndexChar == '\0') // If either one of these is true --> block hasn't been assigned yet
                    continue;

                // calc the offset
                int blockOffset = blockIndex * block_size;
                fseek(sim_disk_fd, blockOffset, SEEK_SET);
                fread(directBlockData, 1, block_size, sim_disk_fd);
                for (int j = 0; j < block_size; j++) {
                    if (directBlockData[j] != '\0') {
                        buf[first] = directBlockData[j];
                        first++;
                    }
                }
            }

            // Handle double indirect blocks
            if (len > direct_enteris + block_size && first < len) {
                int doubleIndirectBlockStart = usedN->getDoubleIndirect(); // Where the double indirect block starts!
                char *doubleIndirectBlockData = readBlock(doubleIndirectBlockStart);
                for (int i = 0; i < block_size && first < len; i++) {
                    char doubleIndirectBlockChar = doubleIndirectBlockData[i];
                    int singleIndirectBlockNum = (int)doubleIndirectBlockChar;
                    if (singleIndirectBlockNum == 0 || doubleIndirectBlockChar == '\0') {
                        continue; // Block hasn't been assigned yet
                    }

                    // Read from the single indirect block
                    int singleIndirectBlockOffset = singleIndirectBlockNum * block_size;
                    fseek(sim_disk_fd, singleIndirectBlockOffset, SEEK_SET);
                    fread(directBlockData, 1, block_size, sim_disk_fd);
                    for (int j = 0; j < block_size && first < len; j++) {
                        if (directBlockData[j] != '\0') {
                            buf[first] = directBlockData[j];
                            first++;
                        }
                    }
                }
            }

            delete[] directBlockData;
        }

        buf[first] = '\0';
        return 0;

    }
};

// Main program for applying the application
int main() {
    int blockSize;
    int direct_entries;
    string fileName;
    string fileName2;
    char str_to_write[DISK_SIZE];
    char str_to_read[DISK_SIZE];
    int size_to_read;
    int _fd;

    fsDisk *fs = new fsDisk();
    int cmd_;
    while(1) { //start the program
        cin >> cmd_;
        switch (cmd_)
        {
            case 0:   // exit
                delete fs;
                exit(0);
                break;

            case 1:  // list-file --> depends in listAll function
                fs->listAll();
                break;

            case 2:    // format
                cin >> blockSize;
                cin >> direct_entries;
                fs->fsFormat(blockSize, direct_entries);
                break;

            case 3:    // creat-file
                cin >> fileName;
                _fd = fs->CreateFile(fileName);
                cout << "CreateFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 4:  // open-file
                cin >> fileName;
                _fd = fs->OpenFile(fileName);
                cout << "OpenFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 5:  // close-file
                cin >> _fd;
                fileName = fs->CloseFile(_fd);
                cout << "CloseFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 6:   // write-file
                cin >> _fd;
                cin >> str_to_write;
                fs->WriteToFile( _fd , str_to_write , strlen(str_to_write) );
                break;

            case 7:    // read-file
                cin >> _fd;
                cin >> size_to_read ;
                fs->ReadFromFile( _fd , str_to_read , size_to_read );
                cout << "ReadFromFile: " << str_to_read << endl;
                break;

            case 8:   // delete file
                cin >> fileName;
                _fd = fs->DelFile(fileName);
                cout << "DeletedFile: " << fileName << " with File Descriptor #: " << _fd << endl;
                break;

            case 9:   // copy file
                cin >> fileName;
                cin >> fileName2;
                fs->CopyFile(fileName, fileName2);
                break;

            case 10:  // rename file
                cin >> fileName;
                cin >> fileName2;
                fs->RenameFile(fileName, fileName2);
                break;

            default:
                break;
        }
    }

}
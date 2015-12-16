#include "processdir.h"

bool isdir(string fname){
    struct stat stat_buf;
    int stat_result=stat(fname.c_str(), &stat_buf);
    return S_ISDIR(stat_buf.st_mode);

}

bool remove_tree(string dir) {

    if (!isdir(dir)){//is it is a simple file try to simply temove it
        if(remove(dir.c_str())){
//            cerr<<"Fail to remove file: "<<dir<<endl;
            return false;
        }else{
//            cout<<"File: "<<dir<<" successfully removed\n";
            return true;
        }
    }

    if(*dir.rbegin() != PATH_SEP)
        dir+=PATH_SEP;

    DIR *pdir = opendir (dir.c_str());

    if (pdir == NULL) {
//        cout<<dir<<" : no such file or directory\n";
        return false;
    }

    struct dirent *pent ;
    string file;

    while (pent = readdir (pdir)) {
        file=dir;
        file+=pent->d_name;
       // cout<<"File to remove: "<<file<<endl;
        if(!strcmp(pent->d_name,".") || !strcmp(pent->d_name,".."))
            continue;
//         cout<<"Removing: "<<file<<endl;
        if (isdir(file)) {
//             cout<<"Removing contents of directory: "<<file<<endl;
              if(!remove_tree(file))
                  return false;

            } else {
                if(remove(file.c_str()))
                    cerr<<"Fail to remove file: "<<file<<endl;
//                else
//                    cout<<"File: "<<file<<" successfully removed\n";
            }

    }


    closedir (pdir);
//    cout<<"Now removing the directory: "<<dir<<endl;
    if (rmdir(dir.c_str())==-1) return false;
//    cout<<"Directory: "<<dir<<" successfully removed\n";
    return true;
}

/*
         int main() {
             string d("mydir");
             if(!remove_tree(d))
                 cerr<<"Error removing: "<<d<<endl;
               return 0;
         }
*/

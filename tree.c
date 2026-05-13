/**
 * 实验：目录树查看器（仿 Linux tree 命令）
 * 学号：2504020304  姓名：陈政勋
 * 说明：请补全所有标记为 TODO 的函数体，不要修改其他代码。
 * 目录树查看器（仿 Linux tree 命令）
 * 完整实现版本（C语言，左孩子右兄弟二叉树）
 * 编译：gcc -o tree tree.c -std=c99
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

// ================== 二叉树结点定义 ==================
typedef struct FileNode {
    char *name;                  // 文件/目录名
    int isDir;                   // 1:目录 0:文件
    struct FileNode *firstChild; // 左孩子：第一个子项
    struct FileNode *nextSibling;// 右兄弟：下一个同层项
} FileNode;

// ================== 函数声明 ==================
FileNode* createNode(const char *name, int isDir);
int cmpNode(const void *a, const void *b);
FileNode* buildTree(const char *path);
void printTree(FileNode *node, const char *prefix, int isLast);
int countNodes(FileNode *root);
int countLeaves(FileNode *root);
int treeHeight(FileNode *root);
void countDirFile(FileNode *root, int *dirs, int *files);
void freeTree(FileNode *root);
char* getBaseName(void);

// ================== 需要补全的函数 ==================

// 创建新结点（分配内存、复制字符串、初始化指针）
FileNode* createNode(const char *name, int isDir) {
    // TODO: 实现
    FileNode *node = (FileNode*)malloc(sizeof(FileNode));
    if (!node) return NULL;
    node->name = (char*)malloc(strlen(name) + 1);
    if (!node->name) {
        free(node);
        return NULL;
    }
    strcpy(node->name, name);
    
    node->isDir = isDir;
    node->firstChild = NULL;
    node->nextSibling = NULL;        
    return node;
}

// 比较函数，用于 qsort 对子项按名称排序
int cmpNode(const void *a, const void *b) {
    // TODO: 实现
    const FileNode *nodeA = *(const FileNode**)a;
    const FileNode *nodeB = *(const FileNode**)b;    
    return strcmp(nodeA->name, nodeB->name);
}

// 递归构建目录树（核心难点）
FileNode* buildTree(const char *path) {
    // TODO: 实现
    DIR *dir = opendir(path);
    if (!dir) return NULL;
    char nodeName[256];
    const char *lastSlash = strrchr(path, '/');
    if (lastSlash == NULL) {
        strncpy(nodeName, path, sizeof(nodeName)-1);
    } else if (lastSlash == path) {
        strcpy(nodeName, "/");
    } else {
        strncpy(nodeName, lastSlash + 1, sizeof(nodeName)-1);
    }
    nodeName[sizeof(nodeName)-1] = '\0';
    FileNode *currentNode = createNode(nodeName, 1);
    if (!currentNode) {
        closedir(dir);                                
    return NULL;
}
    struct dirent *entry;
    FileNode **children = NULL;
    int childCount = 0;
    int childCapacity = 4;
    children = (FileNode**)malloc(childCapacity * sizeof(FileNode*));
    if (!children) {
        freeTree(currentNode);
        closedir(dir);
        return NULL;
    }
    errno = 0;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char fullPath[1024];
        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        struct stat st;
        if (stat(fullPath, &st) != 0) {
            continue;
        }
        FileNode *childNode = NULL;
        if (S_ISDIR(st.st_mode)) {
            childNode = buildTree(fullPath);
        } else if (S_ISREG(st.st_mode)) {
            childNode = createNode(entry->d_name, 0);
        }
        if (childNode) {
            if (childCount >= childCapacity) {
                childCapacity *= 2;
                FileNode **newChildren = (FileNode**)realloc(children, childCapacity * sizeof(FileNode*));
                if (!newChildren) {
                    break;
                }
                children = newChildren;
            }
            children[childCount++] = childNode;
        }
    }
    if (errno != 0) {
        perror("readdir");
    }
    closedir(dir);
    if (childCount > 0) {
        qsort(children, childCount, sizeof(FileNode*), cmpNode);
        
        currentNode->firstChild = children[0];
        FileNode *prev = children[0];
        for (int i = 1; i < childCount; i++) {
            prev->nextSibling = children[i];
            prev = children[i];
        }
    }
    free(children);
    return currentNode;
}                                    
// 树形输出（仿 tree 命令）
void printTree(FileNode *node, const char *prefix, int isLast) {
    // TODO: 实现
    if (!node) return;
    printf("%s", prefix);
    printf(isLast ? "`-- " : "|-- ");
    printf("%s", node->name);
    if (node->isDir) {
        printf("/");
    }
    printf("\n");
    if (!node->firstChild) return;
    FileNode *child = node->firstChild;
    int childCount = 0;
    FileNode *tmp = child;
    while (tmp) {
        childCount++;
        tmp = tmp->nextSibling;
    }
    int idx = 0;
    char newPrefix[1024];
    while (child) {
        idx++;
        int childIsLast = (idx == childCount);
        snprintf(newPrefix, sizeof(newPrefix), "%s%s", prefix, isLast ? "    " : "|   ");
        printTree(child, newPrefix, childIsLast); 
        child = child->nextSibling;
    }
}


// 统计二叉树结点总数
int countNodes(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    int count = 1;
    count += countNodes(root->firstChild);
    count += countNodes(root->nextSibling);        
    return count;
}

// 统计叶子结点数（firstChild == NULL 的结点）
int countLeaves(FileNode *root) {
    // TODO: 实现（递归）
    if (!root) return 0;
    if (root->firstChild == NULL) {
        return 1 + countLeaves(root->nextSibling);
    }
    return countLeaves(root->firstChild) + countLeaves(root->nextSibling);
}        

// 计算二叉树高度（根深度为1，空树高度为0）
int treeHeight(FileNode *root) {
    // TODO: 实现（递归）
     if (!root) return 0;
     int childHeight = treeHeight(root->firstChild);
     int siblingHeight = treeHeight(root->nextSibling);
     return 1 + childHeight > siblingHeight ? (1 + childHeight) : siblingHeight;
 }   

// 统计目录数和文件数（遍历整棵树）
void countDirFile(FileNode *root, int *dirs, int *files) {
    // TODO: 实现（递归）
    if (!root || !dirs || !files) return;
    if (root->isDir) {
        (*dirs)++;
    } else {
        (*files)++;
    }
    countDirFile(root->firstChild, dirs, files);
    countDirFile(root->nextSibling, dirs, files);    
}

// 释放整棵树的内存
void freeTree(FileNode *root) {
    // TODO: 实现（递归释放左右子树，最后释放当前结点）
    if (!root) return;
    freeTree(root->firstChild);
    freeTree(root->nextSibling);
    free(root->name);
    free(root);    
}

// 获取当前工作目录的“基本名称”（用于显示根结点名）
char* getBaseName(void) {
    // TODO: 实现
    char *cwd = getcwd(NULL, 0);
    if (!cwd) return NULL;
    
    char *baseName = NULL;
    const char *lastSlash = strrchr(cwd, '/');
    
    if (lastSlash == NULL) {
        baseName = (char*)malloc(strlen(cwd) + 1);
        if (baseName) strcpy(baseName, cwd);
    } else if (lastSlash == cwd) {
        baseName = (char*)malloc(2);
        if (baseName) strcpy(baseName, "/");
    } else {
        baseName = (char*)malloc(strlen(lastSlash + 1) + 1);
        if (baseName) strcpy(baseName, lastSlash + 1);
    }
    
    // 释放getcwd分配的内存
    free(cwd);    
    // 提示：调用 getcwd(NULL,0) 获取绝对路径，提取最后一个 '/' 之后的部分
    // 注意释放 getcwd 分配的内存
    return baseName;
}

int main(int argc, char *argv[]) {
    char targetPath[1024];
    if (argc >= 2) {
        strncpy(targetPath, argv[1], sizeof(targetPath)-1);
        targetPath[sizeof(targetPath)-1] = '\0';
    } else {
        if (getcwd(targetPath, sizeof(targetPath)) == NULL) {
            perror("getcwd");
            return 1;
        }
    }

    int len = strlen(targetPath);
    if (len > 0 && targetPath[len-1] == '/')
        targetPath[len-1] = '\0';

    struct stat st;
    if (stat(targetPath, &st) != 0) {
        perror("stat");
        return 1;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "错误: %s 不是目录\n", targetPath);
        return 1;
    }

    FileNode *root = buildTree(targetPath);
    if (!root) {
        fprintf(stderr, "无法构建目录树\n");
        return 1;
    }

    // 输出根目录名
    char *displayName = NULL;
    if (argc >= 2) {
        displayName = root->name;
    } else {
        displayName = getBaseName();
    }
    printf("%s/\n", displayName);
    if (argc < 2) free(displayName);

    FileNode *child = root->firstChild;
    int childCount = 0;
    FileNode *tmp = child;
    while (tmp) { childCount++; tmp = tmp->nextSibling; }
    int idx = 0;
    while (child) {
        int isLast = (++idx == childCount);
        printTree(child, "", isLast);
        child = child->nextSibling;
    }

    int dirs = 0, files = 0;
    countDirFile(root, &dirs, &files);
    printf("\n%d 个目录, %d 个文件\n", dirs, files);
    printf("二叉树结点总数: %d\n", countNodes(root));
    printf("叶子结点数: %d\n", countLeaves(root));
    printf("树的高度: %d\n", treeHeight(root));

    freeTree(root);
    return 0;
}

/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     TreeNode *left;
 *     TreeNode *right;
 *     TreeNode(int x) : val(x), left(NULL), right(NULL) {}
 * };
 */
class Solution {
public:
    // isBST() >= 0 means BT rooted at node is BST with x nodes
    // isBST() <  0 means BT rooted at node is not BST, but
    //              has a subtree which is BST with x nodes in it
    int isBST(TreeNode* node, int& minval, int& maxval) {
        if (!node) {
            minval = INT_MAX;
            maxval = INT_MIN;
            return 0;
        }
        int lminval, lmaxval, rminval, rmaxval;
        int lBST = isBST(node->left, lminval, lmaxval);
        int rBST = isBST(node->right, rminval, rmaxval);
        
        // left and right subtree are BSTs
        // and all together make a bigger BST
        if (lBST >= 0 && rBST >= 0 &&
            node->val > lmaxval && node->val < rminval) {
            minval = lBST > 0 ? lminval : node->val;
            maxval = rBST > 0 ? rmaxval : node->val;
            return lBST + rBST + 1;
        }
        // return max size of found BST-subtree
        return -max(abs(lBST), abs(rBST));
    }
    int largestBSTSubtree(TreeNode* root) {
        int minval, maxval;
        return abs(isBST(root, minval, maxval));
    }
};

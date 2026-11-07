#include <iostream>
#include <vector>
using namespace std;

class UnionFind {
private:
    vector<int> parent;  // parent array
    vector<int> rank;    // rank array (tree height)
    int count;           // count of connected components

public:
    // constructor, initialize n elements
    UnionFind(int n) : count(n) {
        parent.resize(n);
        rank.resize(n, 0);
        for (int i = 0; i < n; i++) {
            parent[i] = i; 
        }
    }
    
    // find with path compression
    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]); 
        }
        return parent[x];
    }
    
    // union by rank
    void unite(int x, int y) {
        int rootX = find(x);
        int rootY = find(y);
        
        if (rootX == rootY) return; 
        
        // union by rank
        if (rank[rootX] < rank[rootY]) {
            parent[rootX] = rootY;
        } else if (rank[rootX] > rank[rootY]) {
            parent[rootY] = rootX;
        } else {
            parent[rootY] = rootX;
            rank[rootX]++;
        }
        
        count--; 
    }
    

    bool connected(int x, int y) {
        return find(x) == find(y);
    }
    
    int getCount() const {
        return count;
    }
};


int unionFind_demo() {

    UnionFind uf(5);
    
    cout << "initial state: " << endl;
    cout << "count of connected components: " << uf.getCount() << endl;
    

    cout << "\nbuild friend relationship..." << endl;
    

    uf.unite(0, 1);
    cout << "0 and 1 become friends, count of connected components: " << uf.getCount() << endl;
    

    uf.unite(2, 3);
    cout << "2 and 3 become friends, count of connected components: " << uf.getCount() << endl;
    

    uf.unite(1, 2);
    cout << "1 and 2 become friends, count of connected components: " << uf.getCount() << endl;
    

    cout << "\nquery friend relationship: " << endl;
    cout << "0 and 3 are connected: " << (uf.connected(0, 3) ? "yes" : "no") << endl;
    cout << "0 and 4 are connected: " << (uf.connected(0, 4) ? "yes" : "no") << endl;
    
    return 0;
}
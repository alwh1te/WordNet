#ifndef WORDNET_WORDNET_HPP
#define WORDNET_WORDNET_HPP

#include <iosfwd>
#include <iterator>
#include <limits>
#include <list>
#include <queue>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Digraph {
private:
    int V;
    int E;
    std::vector<std::list<int>> adj;

public:
    Digraph(int V);
    int vertex_count() const;
    int edge_count() const;
    void add_edge(int v, int w);
    const std::list<int> &adjacent(int v) const;
    friend std::ostream &operator<<(std::ostream &os, const Digraph &graph);
};

class ShortestCommonAncestor {
private:
    const Digraph &G;

    struct BFSResult {
        std::vector<int> dist;
        std::vector<int> edgeTo;
        BFSResult(int V);
    };

    BFSResult bfs(int s) const;

public:
    explicit ShortestCommonAncestor(const Digraph &dg);
    unsigned length(unsigned v, unsigned w);
    unsigned ancestor(unsigned v, unsigned w);
    unsigned length_subset(const std::set<unsigned> &subset_a, const std::set<unsigned> &subset_b);
    unsigned ancestor_subset(const std::set<unsigned> &subset_a, const std::set<unsigned> &subset_b);
};

class WordNet {
private:
    std::unordered_map<int, std::vector<std::string>> synset_to_nouns;
    std::unordered_map<std::string, std::vector<int>> noun_to_synsets;
    std::unordered_map<int, std::string> synset_to_gloss;
    Digraph G;

public:
    WordNet(std::istream &synsets, std::istream &hypernyms);

    class Nouns {
    private:
        std::unordered_set<std::string> nouns;

    public:
        class iterator {
        private:
            std::unordered_set<std::string>::const_iterator it;

        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = std::string;
            using difference_type = std::ptrdiff_t;
            using pointer = const std::string *;
            using reference = const std::string &;

            iterator() = default;
            iterator(std::unordered_set<std::string>::const_iterator it);

            iterator &operator++();
            iterator operator++(int);

            bool operator==(const iterator &other) const;
            bool operator!=(const iterator &other) const;

            reference operator*() const;
            pointer operator->() const;
        };

        Nouns(const std::unordered_map<std::string, std::vector<int>> &noun_map);
        iterator begin() const;
        iterator end() const;
    };

    Nouns nouns() const;
    bool is_noun(const std::string &word) const;
    std::string sca(const std::string &noun1, const std::string &noun2) const;
    unsigned distance(const std::string &noun1, const std::string &noun2) const;
};

class Outcast {
private:
    WordNet &wordnet;

public:
    explicit Outcast(WordNet &wordnet);
    std::string outcast(const std::set<std::string> &nouns);
};

#endif// WORDNET_WORDNET_HPP

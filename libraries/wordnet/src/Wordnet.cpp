#include "/libraries/wordnet/include/wordnet/Wordnet.hpp"
#include <regex>
#include <sstream>

Digraph::Digraph(int V) : V(V), E(0), adj(V) {}

int Digraph::vertex_count() const {
    return V;
}

int Digraph::edge_count() const {
    return E;
}

void Digraph::add_edge(int v, int w) {
    adj[v].push_back(w);
    E++;
}

const std::list<int> &Digraph::adjacent(int v) const {
    return adj[v];
}

ShortestCommonAncestor::BFSResult::BFSResult(int V) : dist(V, std::numeric_limits<int>::max()), edgeTo(V, -1) {}

ShortestCommonAncestor::ShortestCommonAncestor(const Digraph &dg) : G(dg) {}

ShortestCommonAncestor::BFSResult ShortestCommonAncestor::bfs(int s) const {
    BFSResult result(G.vertex_count());
    std::queue<int> q;
    result.dist[s] = 0;
    q.push(s);

    while (!q.empty()) {
        int v = q.front();
        q.pop();

        for (int w: G.adjacent(v)) {
            if (result.dist[w] == std::numeric_limits<int>::max()) {
                result.dist[w] = result.dist[v] + 1;
                result.edgeTo[w] = v;
                q.push(w);
            }
        }
    }

    return result;
}

unsigned ShortestCommonAncestor::length(unsigned v, unsigned w) {
    return length_subset({v}, {w});
}

unsigned ShortestCommonAncestor::ancestor(unsigned v, unsigned w) {
    return ancestor_subset({v}, {w});
}

unsigned ShortestCommonAncestor::length_subset(const std::set<unsigned> &subset_a, const std::set<unsigned> &subset_b) {
    std::vector<int> dist_to_a(G.vertex_count(), std::numeric_limits<int>::max());
    std::vector<int> dist_to_b(G.vertex_count(), std::numeric_limits<int>::max());

    for (unsigned v: subset_a) {
        BFSResult bfs_result = bfs(v);
        for (int i = 0; i < G.vertex_count(); ++i) {
            dist_to_a[i] = std::min(dist_to_a[i], bfs_result.dist[i]);
        }
    }

    for (unsigned v: subset_b) {
        BFSResult bfs_result = bfs(v);
        for (int i = 0; i < G.vertex_count(); ++i) {
            dist_to_b[i] = std::min(dist_to_b[i], bfs_result.dist[i]);
        }
    }

    int min_length = std::numeric_limits<int>::max();
    for (int i = 0; i < G.vertex_count(); ++i) {
        if (dist_to_a[i] != std::numeric_limits<int>::max() && dist_to_b[i] != std::numeric_limits<int>::max()) {
            min_length = std::min(min_length, dist_to_a[i] + dist_to_b[i]);
        }
    }

    return min_length;
}

unsigned ShortestCommonAncestor::ancestor_subset(const std::set<unsigned> &subset_a, const std::set<unsigned> &subset_b) {
    std::vector<int> dist_to_a(G.vertex_count(), std::numeric_limits<int>::max());
    std::vector<int> dist_to_b(G.vertex_count(), std::numeric_limits<int>::max());

    for (unsigned v: subset_a) {
        BFSResult bfs_result = bfs(v);
        for (int i = 0; i < G.vertex_count(); ++i) {
            dist_to_a[i] = std::min(dist_to_a[i], bfs_result.dist[i]);
        }
    }

    for (unsigned v: subset_b) {
        BFSResult bfs_result = bfs(v);
        for (int i = 0; i < G.vertex_count(); ++i) {
            dist_to_b[i] = std::min(dist_to_b[i], bfs_result.dist[i]);
        }
    }

    int min_length = std::numeric_limits<int>::max();
    int ancestor = -1;
    for (int i = 0; i < G.vertex_count(); ++i) {
        if (dist_to_a[i] != std::numeric_limits<int>::max() && dist_to_b[i] != std::numeric_limits<int>::max()) {
            int length = dist_to_a[i] + dist_to_b[i];
            if (length < min_length) {
                min_length = length;
                ancestor = i;
            }
        }
    }

    return ancestor;
}

std::string trimString(std::string str) {
    const std::string whiteSpaces = "\r\n\t\f\v";
    size_t first_non_space = str.find_first_not_of(whiteSpaces);
    str.erase(0, first_non_space);
    size_t last_non_space = str.find_last_not_of(whiteSpaces);
    str.erase(last_non_space + 1);
    return str;
}

WordNet::WordNet(std::istream &synsets, std::istream &hypernyms) : G(0) {
    std::string line;
    int max_synset_id = 0;
    while (std::getline(synsets, line)) {
        std::stringstream ss(line);
        std::string id_str, synonyms, gloss;
        std::getline(ss, id_str, ',');
        std::getline(ss, synonyms, ',');
        std::getline(ss, gloss);
        gloss = trimString(gloss);

        try {
            int id = std::stoi(id_str);
            max_synset_id = std::max(max_synset_id, id);

            synset_to_gloss[id] = gloss;

            std::stringstream ss_synonyms(synonyms);
            std::string noun;
            while (ss_synonyms >> noun) {
                noun_to_synsets[noun].push_back(id);
                synset_to_nouns[id].push_back(noun);
            }
        } catch (...) {}
    }

    G = Digraph(max_synset_id + 1);

    while (std::getline(hypernyms, line)) {
        std::stringstream ss(line);
        std::string id_str;
        std::getline(ss, id_str, ',');
        try {
            int id = std::stoi(id_str);
            std::string hypernym_id_str;
            while (std::getline(ss, hypernym_id_str, ',')) {
                int hypernym_id = std::stoi(hypernym_id_str);
                G.add_edge(id, hypernym_id);
            }
        } catch (...) {}
    }
    nounsClass = Nouns(noun_to_synsets);
}

WordNet::Nouns::Nouns(const std::unordered_map<std::string, std::vector<int>> &noun_map) {
    for (const auto &pair: noun_map) {
        nouns.insert(pair.first);
    }
}

WordNet::Nouns::iterator::iterator(std::unordered_set<std::string>::const_iterator it) : it(it) {}

WordNet::Nouns::iterator &WordNet::Nouns::iterator::operator++() {
    ++it;
    return *this;
}

WordNet::Nouns::iterator WordNet::Nouns::iterator::operator++(int) {
    iterator temp = *this;
    ++it;
    return temp;
}

bool WordNet::Nouns::iterator::operator==(const iterator &other) const {
    return it == other.it;
}

bool WordNet::Nouns::iterator::operator!=(const iterator &other) const {
    return it != other.it;
}

WordNet::Nouns::iterator::reference WordNet::Nouns::iterator::operator*() const {
    return *it;
}

WordNet::Nouns::iterator::pointer WordNet::Nouns::iterator::operator->() const {
    return &(*it);
}

WordNet::Nouns::iterator WordNet::Nouns::begin() const {
    return iterator(nouns.cbegin());
}

WordNet::Nouns::iterator WordNet::Nouns::end() const {
    return iterator(nouns.cend());
}

WordNet::Nouns WordNet::nouns() const {
    return nounsClass;
}

bool WordNet::is_noun(const std::string &word) const {
    return noun_to_synsets.find(word) != noun_to_synsets.end();
}

std::string WordNet::sca(const std::string &noun1, const std::string &noun2) const {
    if (!is_noun(noun1) || !is_noun(noun2)) {
        throw std::invalid_argument("Noun not found in WordNet");
    }

    std::set<unsigned> synset_ids1(noun_to_synsets.at(noun1).begin(), noun_to_synsets.at(noun1).end());
    std::set<unsigned> synset_ids2(noun_to_synsets.at(noun2).begin(), noun_to_synsets.at(noun2).end());

    ShortestCommonAncestor sca(G);
    unsigned ancestor_id = sca.ancestor_subset(synset_ids1, synset_ids2);
    if (synset_to_gloss.find(ancestor_id) != synset_to_gloss.end()) {
        return (synset_to_gloss.at(ancestor_id));
    } else {
        throw std::runtime_error("Ancestor id not found in synset_to_gloss map");
    }
}

unsigned WordNet::distance(const std::string &noun1, const std::string &noun2) const {
    if (!is_noun(noun1) || !is_noun(noun2)) {
        throw std::invalid_argument("Noun not found in WordNet");
    }

    std::set<unsigned> synset_ids1(noun_to_synsets.at(noun1).begin(), noun_to_synsets.at(noun1).end());
    std::set<unsigned> synset_ids2(noun_to_synsets.at(noun2).begin(), noun_to_synsets.at(noun2).end());

    ShortestCommonAncestor sca(G);
    return sca.length_subset(synset_ids1, synset_ids2);
}


Outcast::Outcast(WordNet &wordnet) : wordnet(wordnet) {}

std::string Outcast::outcast(const std::set<std::string> &nouns) {
    if (nouns.size() <= 2) {
        return "";
    }

    std::unordered_map<std::string, int> dist_sums;
    int max_distance = -1;
    std::string outcast_word;
    bool is_unique_outcast = false;

    for (const std::string &noun: nouns) {
        int dist_sum = 0;

        for (const std::string &other_noun: nouns) {
            if (noun != other_noun) {
                dist_sum += wordnet.distance(noun, other_noun);
            }
        }

        dist_sums[noun] = dist_sum;

        if (dist_sum > max_distance) {
            max_distance = dist_sum;
            outcast_word = noun;
            is_unique_outcast = true;
        } else if (dist_sum == max_distance) {
            is_unique_outcast = false;
        }
    }

    if (is_unique_outcast) {
        return outcast_word;
    }

    return "";
}

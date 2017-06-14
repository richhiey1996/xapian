/** @file cluster.h
 *  @brief Cluster API
 */
/* Copyright (C) 2010 Richard Boulton
 * Copyright (C) 2016 Richhiey Thomas
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef XAPIAN_INCLUDED_CLUSTER_H
#define XAPIAN_INCLUDED_CLUSTER_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
#error "Never use <xapian/cluster.h> directly; include <xapian.h> instead."
#endif

#include <xapian/mset.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <unordered_map>
#include <vector>

namespace Xapian {

/** Class representing a set of documents in a cluster
 */
class XAPIAN_VISIBILITY_DEFAULT DocumentSet {
  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /** Copying is allowed.  The internals are reference counted, so
     *  copying is cheap.
     *
     *  @param other	The object to copy.
     */
    DocumentSet(const DocumentSet &other);

    /** Assignment is allowed.  The internals are reference counted,
     *  so assignment is cheap.
     *
     *  @param other	The object to copy.
     */
    void operator=(const DocumentSet &other);

    /// Constructor
    DocumentSet();

    /// Destructor
    ~DocumentSet();

    /// Return the size of the DocumentSet
    unsigned int size() const;

    /// Return the Document in the DocumentSet at index i
    Xapian::Document operator[](Xapian::doccount i);

    /** Add a new Document to the DocumentSet
     *
     *  @param doc	Document object that is to be added to
     *			the DocumentSet
     */
    void add_document(const Document &doc);
};

/** Base class for TermListGroup
 *  Stores and provides terms that are contained in a document and
 *  their respective term frequencies
 */
class XAPIAN_VISIBILITY_DEFAULT FreqSource
    : public Xapian::Internal::opt_intrusive_base {

  private:

    /// Don't allow assignment.
    void operator=(const FreqSource &);

    /// Don't allow copying.
    FreqSource(const FreqSource &);

  public:

    /// Constructor
    FreqSource() {}

    /// Destructor
    virtual ~FreqSource();

    /** Return the term frequency of a particular term 'tname'
     *
     *  @param tname	The term for which we return the frequency value
     */
    virtual doccount get_termfreq(const std::string &tname) const = 0;

    /// Return the number of documents within the MSet
    virtual doccount get_doccount() const = 0;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated FreqSource
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    FreqSource * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated FreqSource
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const FreqSource * release() const {
	opt_intrusive_base::release();
	return this;
    }
};

/** A class for dummy frequency source for construction of termlists
 *  This returns 1 as the term frequency for any term
 */
class XAPIAN_VISIBILITY_DEFAULT DummyFreqSource : public FreqSource {

  public:

    /// Return the value 1 as a dummy term frequency
    doccount get_termfreq(const std::string &) const;

    doccount get_doccount() const;
};

/** A class for construction of termlists which store the terms for a
 *  document along with the number of documents it indexes i.e. term
 *  frequency
 */
class XAPIAN_VISIBILITY_DEFAULT TermListGroup : public FreqSource {

  private:

    /** Map of the terms and its corresponding term frequencies.
     *  The term frequency of a term stands for the number of documents it indexes
     */
    std::unordered_map<std::string, doccount> termfreq;

    /// Number of documents added to the termlist
    doccount docs_num;

    /** Add a single document and calculates its corresponding term frequencies
     *
     *  @param doc	Adds a document and updates the TermListGroup based on the
     *			terms found in the document
     */
    void add_document(const Document &doc);

  public:
    /** Constructor
     *
     *  @params docs	MSet object used to construct the TermListGroup
     */
    explicit TermListGroup(const MSet &docs);

    /** Return the number of documents that the term 'tname' exists in
     *  or the number of documents that a certain term indexes
     *
     *  @param tname	The term for which we return the frequency value
     */
    doccount get_termfreq(const std::string &tname) const;

    doccount get_doccount() const;
};

/** Abstract class representing a point in the VSM
 */
class XAPIAN_VISIBILITY_DEFAULT PointType
    : public Xapian::Internal::opt_intrusive_base {

  protected:

    /** Implement a map to store the terms within a document
     *  and their pre-computed TF-IDF values
     */
    std::unordered_map<std::string, double> values;

    /// Store the squared magnitude of the PointType
    double magnitude;

    /** Set the value 'value' to the mapping of a term
     *
     *  @param term	Term for which the value is supposed
     *			to be changed
     *  @param value	The value to which the mapping of the
     *			term is to be set
     */
    void set_value(const std::string &term, double value);

  public:
    /// Constructor
    PointType() { magnitude = 0; }

    /// Return a TermIterator to the beginning of the termlist
    TermIterator termlist_begin() const;

    /// Return a TermIterator to the end of the termlist
    TermIterator termlist_end() const {
	return TermIterator(NULL);
    }

    /** Validate whether a certain term exists in the termlist
     *  or not by performing a lookup operation in the existing values
     *
     *  @param term	Term which is to be searched
     */
    bool contains(const std::string &term) const;

    /** Return the TF-IDF weight associated with a certain term
     *
     *  @param term	Term for which TF-IDF weight is returned
     */
    double get_value(const std::string &term) const;

    /** Add the value 'value' to the mapping of a term
     *
     *  @param term	Term to which the value is to be added
     *  @param value	Value which has to be added to the existing
     *			mapping of the term
     */
    void add_value(const std::string &term, double value);

    /// Return the pre-computed squared magnitude
    double get_magnitude() const;

    /// Return the size of the termlist
    int termlist_size() const;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated PointType
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    PointType * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated PointType
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const PointType * release() const {
	opt_intrusive_base::release();
	return this;
    }
};

/** Class to represent a document as a point in the Vector Space
 *  Model
 */
class XAPIAN_VISIBILITY_DEFAULT Point : public PointType {

  private:

    /// The document which is being represented by the Point
    Document doc;

  public:

    /** Initialize the point with terms and corresponding term weights
     *
     *  @param tlg	TermListGroup object which provides the term
     *			frequencies which is used for TF-IDF weight calulations
     *  @param doc	The Document object over which the Point object
     *			will be initialized
     */
    void initialize(const TermListGroup &tlg, const Document &doc);

    /// Returns the document corresponding to this Point
    Document get_document() const;
};

/** Class to represent cluster centroids in the vector space
*/
class XAPIAN_VISIBILITY_DEFAULT Centroid : public PointType {

  public:

    // Constructor
    Centroid();

    /** Constructor with Point argument
     *
     *  @param x	Point object to which Centroid object is
     *			initialized. The document vector and the
     *			magnitude are made equal
     */
    explicit Centroid(Point &x);

    /** Divide the weight of terms in the centroid by 'size'
     *
     *  @param size	Value by which Centroid document vector is
     *			divided
     */
    void divide(double size);

    /// Clear the terms and corresponding values of the centroid
    void clear();

    /// Recalculate the magnitude of the centroid
    void recalc_magnitude();
};

/** Class to represents a Cluster which contains Points and
 *  of the Cluster
 */
class XAPIAN_VISIBILITY_DEFAULT Cluster {

  public:
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /** Copying is allowed.  The internals are reference counted, so
     *  copying is cheap.
     *
     *  @param other	The object to copy.
     */
    Cluster(const Cluster &other);

    /** Assignment is allowed.  The internals are reference counted,
     *  so assignment is cheap.
     *
     *  @param other	The object to copy.
     */
    Cluster& operator=(const Cluster &other);

    /** Constructor
     *
     *  @param centroid_	The centroid of the cluster object is
     *				assigned to centroid_
     */
    Cluster(const Centroid &centroid_);

    /// Constructor
    Cluster();

     /// Destructor
    ~Cluster();

    /// Returns size of the cluster
    Xapian::doccount size() const;

    /** Add a document to the Cluster
     *
     *  @param doc	The Point object representing the document which
     *			needs to be added to the cluster
     */
    void add_point(const Point &doc);

    /// Clear the cluster values
    void clear();

    /** Return the point at the given index in the cluster
     *
     *  @param index	Index of the Point within the cluster
     */
    Point get_index(unsigned int index) const;

    /// Return the documents that are contained within the cluster
    DocumentSet get_documents();

    /// Return the current centroid of the cluster
    Centroid& get_centroid() const;

    /** Set the centroid of the Cluster to centroid_
     *
     *  @param centroid_	Centroid object for the Cluster
     */
    void set_centroid(const Centroid &centroid_);

    /** Recalculate the centroid of the Cluster after each iteration
     *  of the KMeans algorithm by taking the mean of all document vectors (Points)
     *  that belong to the Cluster
     */
    void recalculate();
};

/** Class for storing the results returned by the Clusterer
 */
class XAPIAN_VISIBILITY_DEFAULT ClusterSet {

  public:

    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /** Copying is allowed.  The internals are reference counted, so
     *  copying is cheap.
     *
     *  @param other	The object to copy.
     */
    ClusterSet(const ClusterSet &other);

    /** Assignment is allowed.  The internals are reference counted,
     *  so assignment is cheap.
     *
     *  @param other	The object to copy.
     */
    ClusterSet& operator=(const ClusterSet &other);

    /// Constructor
    ClusterSet();

    /// Destructor
    ~ClusterSet();

    /** Add a cluster to the cluster set
     *
     *  @param c	Cluster object which is to be added
     *			to the ClusterSet
     */
    void add_cluster(const Cluster &c);

    /** Return the Cluster at position 'index'
     *
     *  @param index	The index of the required Cluster within the
     *			ClusterSet
     */
    Cluster get_cluster(unsigned int index) const;

    /** Add the point the the cluster at position 'index'
     *
     *  @param x	Point object which needs to be added to
     *			a Cluster within the ClusterSet
     *  @param index	Index of the Cluster within the ClusterSet to
     *			which the Point is to be added
     */
    void add_to_cluster(const Point &x, unsigned int index);

    /// Return the number of clusters
    Xapian::doccount size() const;

    /// Return the cluster at index 'i'
    Cluster& operator[](Xapian::doccount i);

    /// Clear all the clusters in the ClusterSet
    void clear_clusters();

    /** Recalculate the centroids for all the centroids
     *  in the ClusterSet
     */
    void recalculate_centroids();
};

/** Base class for calculating the similarity between documents
 */
class XAPIAN_VISIBILITY_DEFAULT Similarity {

  public:

    /// Destructor
    virtual ~Similarity();

    /** Calculates the similarity between the two documents
     *
     *  @param a	First point object for distance calculation
     *  @param b	Second point object for distance calculation
     */
    virtual double similarity(const PointType &a, const PointType &b) const = 0;

    /// Returns description of the similarity metric being used
    virtual std::string get_description() const = 0;
};

/** Class for calculating the cosine distance between two documents
 */
class XAPIAN_VISIBILITY_DEFAULT CosineDistance : public Similarity {

  public:

    /** Calculates and returns the cosine similarity using the
     *  formula  cos(theta) = a.b/(|a|*|b|)
     */
    double similarity(const PointType &a, const PointType &b) const;

    /// Returns the description of Cosine Similarity
    std::string get_description() const;
};

/** Class representing an abstract class for a clusterer to be implemented
 */
class XAPIAN_VISIBILITY_DEFAULT Clusterer
    : public Xapian::Internal::opt_intrusive_base {

  public:

    /// Destructor
    virtual ~Clusterer();

    /** Implement the required clustering algorithm in the subclass and
     *  and return clustered output as ClusterSet
     *
     *  @param mset	The MSet object which contains the documents to be
     *			clustered
     */
    virtual ClusterSet cluster(const MSet &mset) = 0;

    /// Returns a description of the clusterer being used
    virtual std::string get_description() const = 0;

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated Clusterer
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    Clusterer * release() {
	opt_intrusive_base::release();
	return this;
    }

    /** Start reference counting this object.
     *
     *  You can hand ownership of a dynamically allocated Clusterer
     *  object to Xapian by calling release() and then passing the object to a
     *  Xapian method.  Xapian will arrange to delete the object once it is no
     *  longer required.
     */
    const Clusterer * release() const {
	opt_intrusive_base::release();
	return this;
    }
};

/** Round Robin clusterer:
 *  This clusterer is a minimal clusterer which will cluster documents as -
 *  ith document goes to the (i % k)th cluster where k is the number of clusters and
 *  0 <= i < N; where N is the number of documents
 */
class XAPIAN_VISIBILITY_DEFAULT RoundRobin : public Clusterer {

    /// Number of clusters to be formed by the clusterer
    unsigned int num_of_clusters;

  public:

    /** Constructor
     *
     *  @param num_of_clusters_		Number of required clusters
     */
    RoundRobin(unsigned int num_of_clusters_) : num_of_clusters(num_of_clusters_) {}

    /// Implements the RoundRobin clustering
    ClusterSet cluster(const MSet &mset);

    /// Returns the description of the clusterer
    std::string get_description() const;
};

/** Kmeans clusterer:
 *  This clusterer implements the K-Means clustering algorithm
 */
class XAPIAN_VISIBILITY_DEFAULT KMeans : public Clusterer {

    /// Contains the initialized points that are to be clustered
    std::vector<Point> docs;

    /// Specifies that the clusterer needs to form 'k' clusters
    unsigned int k;

    /// Specifies the maximum number of iterations that KMeans will have
    unsigned int max_iters;

    /** Initialize 'k' clusters by randomly selecting 'k' centroids
     *  and assigning them to different clusters
     *
     *	@param cset	ClusterSet object to be initialized by assigning
     *			centroids to each cluster
     */
    void initialize_clusters(ClusterSet &cset);

    /** Initialize the 'Points' to be fed into the Clusterer with the DocumentSource.
     *  The TF-IDF weights for the points are calculated and stored within the
     *  Points to be used later during distance calculations
     *
     *  @param docs	MSet object containing the documents which will be
     *			used to create document vectors that are represented
     *			as Point objects
     */
    void initialize_points(const MSet &docs);

  public:

    /** Constructor specifying number of clusters and maximum iterations
     *
     *  @param k_		Number of required clusters
     *  @param max_iters_	The maximum number of iterations for which KMeans
     *				will run if it doesn't converge
     */
    explicit KMeans(unsigned int k_, unsigned int max_iters_ = 0);

    /// Implements the KMeans clustering algorithm
    ClusterSet cluster(const MSet &mset);

    /// Returns the description of the clusterer
    std::string get_description() const;
};
}
#endif

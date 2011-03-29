/**
 * @file UAutomata.h
 * @brief  Definition of class UAutomata, it implements leveinshtain universal automata with  q-errors(edit distance is q)
 * @author Jinli Liu
 * @date Sep 15,2009
 * @version v1.0
 */
#ifndef __UAUTOMATA_H_
#define __UAUTOMATA_H_
#include "UAutomataTypes.h"

namespace sf1r{
    class UAutomata{
    public:

        UAutomata(int ed);
         
        ~ UAutomata(){}
        /**
	     * @brief The main interface to construct universal automaton
	     * @param stateVector  state vector used to store states generated by universal automata
	     * @param autoMaton    hash_map used to store universal automata transition(source stateNo && label) and target stateNo.
	     */
        void buildAutomaton(vector<STATE>& stateVector, rde::hash_map<rde_tran,int>& autoMaton);
        
        
    protected:  
      
        /**
	     * @brief this function is used to check if one position can be deleted  in order to make the deterministic automata
	     * @param p1       first position
	     * @param p2       seconde position
	     * @return true if p1 subsumes p2, otherwise false 
	     */
        bool lessThanSubsume(const Position &p1, const Position &p2);
        
        
        /**
	     * @brief This function is used for checking whether the length of the word b1 b2 ...bk is suitable to define the transition function Delta()
	     * @param k       length of word b1 b2 ...bk
	     * @param st      current state
	     * @return true if k is suitable to define Delta(),otherwise false. 
	     */
        bool lengthCoversAllPositions( int k, const STATE &st);      
                                                 
        /**
	     * @brief This function is used to convert the non-final states into final state
	     * @param st      non-final state
	     * @param k       length of word b1 b2 ...bk	
	     * @return final state
	     */
        STATE  convertToFinalState(  const STATE& st, int k );
        
        
        /**
	     * @brief This function is used to find the right most position(element) of one state
	     * @param st      current state
	     * @return right most position
	     */
        Position findRightMostPosition( const STATE &st);
        
        
         /**
	     * @brief This function is used to check if one position is good candidate
	     * @param pos     current position
	     * @param k       length of word b1 b2 ...bk	
	     * @return true if it is good candidate,otherwise false.
	     */
        bool isGoodCandidate( const Position &pos, int k );
        
         
          /**
	     * @brief This function is used to find the deterministic automaton preparing for find elementary state in Delta_E()
	     * @param pt       current point
	     * @param b       symbol used for transition
	     * @return Points  
	     */
        Points findDeterimineState( const Point& pt, const std::string& b);
        
                                                                 
        /**
	     * @brief This function is used to compute the characteristic vetor determined by Posistion pos and symbol b
	     * @param pos     current position
	     * @param b       symbol of b1 b2 ...bk	
	     * @return characteristic vetor.
	     */
        std::string computeCharVector(const Position &pos, const std::string& b ) ;
        
       
         /**
	     * @brief This function is used to find elementary state preparing for refined by Delta()
	     * @param p       current position
	     * @param b       symbol used for transition
	     * @return elementary state from current position p and given symbol b
	     */
        STATE findElementaryState(  const Position &p, const string &b );
        
                
         /**
	     * @brief This function is used to find next state(nextSt) from given state st and symbol b
	     * @param st       current state
	     * @param b       symbol used for transition
	     * @return the size of nextSt
	     */
        STATE findNextState(const STATE& st, const string &b);        
        
        /**
	     * @brief This function is used to  generate the combination made of {0,1}*
	     * @param combination   vector used to store {0,1}*
	     * @param candidate   current result
	     * @param position    current position,start from 0
	     */
        void initCombination(vector<std::string>& combination, std::string& candidate, unsigned int position);
        
    private:
        /**
	     * @brief hash one state into integer in order to speed the access rate
	     */
         hash_map<STATE, int, izenelib::util::HashFunctor<STATE> > stateMap;
        /**
	     * @brief this vector is used to store all of combination made up with {0.1}*
	     */
         vector<std::string> combination;
         /**
	     * @brief the maximum edit distance
	     */
         int ed;
       
    };
}/*end of sf1r*/
#endif



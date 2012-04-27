/**
 * @file VisitMatrix.h
 * @brief update covisit matrix of visit items
 * @author Jun Jiang
 * @date 2012-01-19
 */

#ifndef VISIT_MATRIX_H
#define VISIT_MATRIX_H

#include "RecommendMatrix.h"
#include <aggregator-manager/UpdateRecommendBase.h>

namespace sf1r
{

class VisitMatrix : public RecommendMatrix
{
public:
    VisitMatrix(UpdateRecommendBase& matrix)
        : matrix_(matrix)
    {}

    virtual void update(
        const std::list<itemid_t>& oldItems,
        const std::list<itemid_t>& newItems
    )
    {
        bool result = true;
        matrix_.updateVisitMatrix(oldItems, newItems, result);
    }

private:
    UpdateRecommendBase& matrix_;
};

} // namespace sf1r

#endif // VISIT_MATRIX_H

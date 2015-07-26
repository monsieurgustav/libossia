#include <QtTest>
#include "../ForwardDeclaration.h"
#include <iostream>

using namespace OSSIA;

void constraint_callback(const TimeValue& position, const TimeValue& date, std::shared_ptr<State> state)
{
    ;
}

void event_callback(TimeEvent::Status newStatus)
{
    ;
}

class ScenarioTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    
    /*! test life cycle and accessors functions */
    void test_basic()
    {
        auto scenario = Scenario::create();
        QVERIFY(scenario != nullptr);
        
        QVERIFY(scenario->getStartState() != nullptr);
        QVERIFY(scenario->getEndState() != nullptr);
        QVERIFY(scenario->getParentTimeConstraint() == nullptr);
        
        QVERIFY(scenario->getStartNode() != nullptr);
        QVERIFY(scenario->getEndNode() != nullptr);
        QVERIFY(scenario->isKiller() == false);
        
        scenario->setKiller(true);
        QVERIFY(scenario->isKiller() == true);
    }
    
    /*! test edition functions */
    void test_edition()
    {
        auto scenario = Scenario::create();
        
        auto start_node = scenario->getStartNode();
        auto start_event = *(start_node->emplace(start_node->timeEvents().begin(), &event_callback));
        
        auto end_node = TimeNode::create();
        auto end_event = *(end_node->emplace(end_node->timeEvents().begin(), &event_callback));
        
        auto constraint = TimeConstraint::create(&constraint_callback, start_event, end_event, 1000.);
        
        //! \todo how to verify something here ?
        scenario->addTimeConstraint(constraint);
        
        //! \todo how to verify something here ?
        scenario->removeTimeConstraint(constraint);

        auto lonely_node = TimeNode::create();
        
        //! \todo how to verify something here ?
        scenario->addTimeNode(lonely_node);
        
        //! \todo how to verify something here ?
        scenario->removeTimeNode(lonely_node);

        //! \todo how to verify something here ?
        auto scenario_copy = scenario->clone();
    }

    /*! test execution functions */
    void test_execution()
    {
        //! \todo test state()
    }
};

QTEST_APPLESS_MAIN(ScenarioTest)

#include "ScenarioTest.moc"


/**
 * @file demo.cpp
 * @author Jacob Beck <jacob.beck27@gmail.com>
 **/

#include <iostream>
#include <string>
#include <sstream>
#include "madara/knowledge/KnowledgeBase.h"
#include "madara/utility/Utility.h"

const int MAX_HOSTS = 10;

int main (int argc, char * argv[])
{
  // Create knowledge base
  madara::knowledge::KnowledgeBase knowledge;
  
  // Check command line arguments for id
  if (argc >= 2)
  {
    // save the first argument into an integer
    madara::knowledge::KnowledgeRecord::Integer new_id;
    std::istringstream buffer (argv[1]);
    buffer >> new_id;
    
    // Setup TCP-based ZMQ transport
    madara::transport::QoSTransportSettings settings;
    settings.type = madara::transport::ZMQ;
    
    // Our host name needs to come first
    std::ostringstream hostname;
    hostname << "tcp://127.0.0.1:" << (30000 + new_id);
    settings.hosts.push_back(hostname.str () );
    hostname.str ("");
    
    // Add the rest of the agents
    for (int i = 0; i < MAX_HOSTS; i++)
    {
      if (i == new_id)
        continue;
      
      hostname << "tcp://127.0.0.1:" << (30000 + i);
      settings.hosts.push_back(hostname.str () );
      hostname.str ("");
    }

    // Apply transport settings to knowledge base
    knowledge.attach_transport (std::to_string (new_id), settings);

    /**
     * Update the knowledge base to include our .id. All variables are zero
     * by default in the knowledge base.
     **/
    knowledge.set (".id", new_id);
  }
  else
  {
    knowledge.evaluate ("#print ('ID argument is required\n') ");
    return 1;
  }
  
  
  while (1)
  {
    knowledge.evaluate (
      /**
       * In KaRL, a variable without a period in front of it is meant to be
       * disseminated to every other reasoning agent in the network who is
       * in our domain. The first thing we do in this logic is to tell other
       * processes that we are online.
       **/
      "agent{.id}.online = 1;"

      /**
       * We next set a variable called .cur_agents to 0. We'll use this to
       * count the number of agents online right now.
       **/
      ".cur_agents = 0;"

      /**
       * The second step in this logic is to count the number of online agents.
       * The following code is a for loop that iterates a variable called .i
       * from 0 to 10 (exclusively, meaning we really only go from 0 to 9).
       * In this loop, we count the number of online agents.
       **/
       ".i [0->10) (.cur_agents += agent{.i}.online);"

       /**
        * Another interesting metric to keep track of is the maximum number of
        * agents that were ever online. We do that by exploiting the power of
        * the semicolon operator, which separates expressions and also returns
        * the maximum of the two expressions.
        **/
        ".max_agents = (.max_agents ; .cur_agents);"

        /**
         * Last, let's print out both the current agents and the max agents.
         * Note that we've cut this single print statement into two lines to
         * keep us within 80 column boundaries. We're also showcasing string
         * concatenation within the engine.
         **/
         "#print ('Cur agents online: {.cur_agents}  ' +"
         "        'Max agents online {.max_agents}\n')"
    );

    // Sleep for a second so we limit the number of printouts
    madara::utility::sleep (1);
  }
  
 
  /**
   * Let everyone know that we are going offline and also do one last
   * agent count and print the results. We'll showcase a second way to
   * print information in an evaluation here. Each evaluate call can
   * be passed settings that dictate how the evaluation takes place.
   * In this case, we set the EvalSettings class to include a post
   * print statement that prints the cur and max agents
   **/
  madara::knowledge::EvalSettings eval_settings;
  eval_settings.post_print_statement = 
    "Cur agents online: {.cur_agents}  Max agents online: {.max_agents}\n";

  /**
   * Evaluate the number of agents one last time before we return to the OS
   **/
  knowledge.evaluate (
"agent{.id}.online = 0; \
.cur_agents = 0; \
.i [0->10) (.cur_agents += agent{.i}.online); \
.max_agents = (.max_agents ; .cur_agents)"
  , eval_settings);

  return 0;
}

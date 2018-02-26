
#ifndef TURNRIGHTSEGMENT_HPP
#define TURNRIGHTSEGMENT_HPP

#include "turnsegment.hpp"

class TurnRightSegment: public TurnSegment {

    public:

        TurnRightSegment(const glm::vec3& position, float orientation,
            int input);
        virtual ~TurnRightSegment();

        /* Get the next segment's position (the connection point)
           Parameters:
             * output: the index of the output to use.
             * pos: output position.
             * orient: output orientation.
        */
        virtual void get_output(
            int output, glm::vec3& pos, float& orient) const;

    private:

        // Initialize the geometry
        static void init_geometry_static();

        // Geometry definition
        static gl_vertex_t vertices[];
        static GLushort indices[];

        // true if the road geometry has been defined, false otherwise. Used to
        // initialize the geometry only with the first instance.
        static bool geometry_defined;

};

#endif


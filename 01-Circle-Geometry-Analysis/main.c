#include<stdio.h>
#include<math.h>

/* Constants for mathematical calculations and precision */
#define PI 3.14159265358979323846
#define EPSILON 1e-9

int main ()
{
    double x1, y1, r1;
    double x2, y2, r2;
    double dx, dy, d, overlap;

    /* Input handling for Circle #1 */
    printf("Enter circle #1 parameters:\n");
    if (scanf("%lf %lf %lf", &x1, &y1, &r1)!= 3 || r1 <= 0) {
        printf("Invalid input.\n");
        return 0;
    }

    /* Input handling for Circle #2 */
    printf("Enter circle #2 parameters:\n");
    if (scanf("%lf %lf %lf", &x2, &y2, &r2)!= 3 || r2 <= 0) {
        printf("Invalid input.\n");
        return 0;
    }

    /* Calculate Euclidean distance between the two centers */
    dx = x2 - x1;
    dy = y2 - y1;
    d = sqrt(dx * dx + dy * dy);

    /* Scale epsilon based on the largest radius to ensure relative precision */
    double scale = fmax(r1, r2);
    double relative_epsilon = EPSILON * scale;

    /* Case 1: Identical Circles */
    if (fabs(d) < relative_epsilon && fabs(r1 - r2) < relative_epsilon){
        overlap = PI * r1 * r1;
        printf("The circles are identical, overlap: %lf\n", overlap);
        return 0;
    }

    /* Case 2: Circles are far apart (Disjoint) */
    if (d > r1 + r2 + relative_epsilon) {
        printf("The circles lie outside each other, no overlap.\n");
        return 0;
    }

    /* Case 3: External Touch */
    if (fabs(d - (r1 + r2)) < relative_epsilon) {
        printf("External touch, no overlap.\n");
        return 0;
    }

    /* Case 4: Circle 2 is fully inside Circle 1 */
    if (d + r2 < r1 - relative_epsilon) {
        overlap = PI * r2 * r2;
        printf("Circle #2 lies inside circle #1, overlap: %lf\n", overlap);
        return 0;
    }

    /* Case 5: Circle 1 is fully inside Circle 2 */
    if (d + r1 < r2 - relative_epsilon) {
        overlap = PI * r1 * r1;
      printf("Circle #1 lies inside circle #2, overlap: %lf\n", overlap);
        return 0;
    }

    /* Case 6: Internal Touch */
    if(fabs(d - fabs(r1 - r2)) < relative_epsilon){
        double smaller = (r1 < r2) ? r1 : r2;
        overlap = PI * smaller * smaller;
        if (r1 > r2)
            printf("Internal touch, circle #2 lies inside circle #1, overlap: %lf\n",overlap);
        else
            printf("Internal touch, circle #1 lies inside circle #2, overlap: %lf\n",overlap);
        return 0;
    }

    /* Case 7: Circles Intersect 
       Calculating the area of overlap using the circular segment formula */
    {
        double case1, case2, case3;
        case1 = (r1 * r1) * acos((d * d + r1 * r1 - r2 * r2) / (2 * d * r1));
        case2 = (r2 * r2) * acos((d * d + r2 * r2 - r1 * r1) / (2 * d * r2));
        case3 = 0.5 * sqrt((-d + r1 + r2) * (d + r1 - r2) * (d - r1 + r2) * (d + r1 + r2));
        overlap = case1 + case2 - case3;
    }
    printf("The circles intersect, overlap: %lf\n", overlap);

    return 0;
}

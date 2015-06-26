#include "proposal.hh"
#include <Rcpp.h>

#include <boost/numeric/ublas/matrix.hpp>
namespace flu {
    /**
     * \brief Functions to keep track of proposal distribution
     */
    namespace proposal {
        bu::matrix<double> cholesky_factorization(
                const bu::matrix<double> &A)
        {
            assert( A.size1() == A.size2() );

            bu::matrix<double> res( A.size1(), A.size2() );
            //std::fill( res.begin1(), res.end1(), 0 );
            for(size_t i=0;i<A.size1();i++)
            {
                for(size_t j=0;j<A.size2();j++)
                {
                    res(i,j) = 0;
                }
            }

            for(size_t i=0;i<A.size1();i++)
            {
                for(size_t j=i;j<A.size2();j++)
                {
                    double sum_L2=A(i,j);
                    for(size_t k=0;k<i;k++)
                        sum_L2-=res(i,k)*res(j,k);
                    if(i==j)
                        res(i,i)=sqrt(sum_L2);
                    else
                        res(j,i)=sum_L2/res(i,i);
                }
            }
            return res;
        }

        bu::matrix<double> update_sum_corr(bu::matrix<double> &&corr, 
                const parameter_set &par )
        {
            //vectorize parameters
            std::vector<double> v = {
                par.epsilon[0], par.epsilon[2], par.epsilon[4],
                par.psi, par.transmissibility,
                par.susceptibility[0], par.susceptibility[3],
                par.susceptibility[6], par.init_pop
            };

            /*first line*/
            for (size_t i = 0; i < corr.size1(); ++i)
            {
                for (size_t j = 0; j < corr.size2(); ++j )
                {
                    if (j<=i)
                        corr(i,j) += v[i]*v[j];
                    else
                        corr(i,j) = corr(j,i);
                }
            }
            return corr;
        }

        proposal_state_t update( proposal_state_t&& state,
                const parameter_set &parameters,
                int k ) 
        {
            /*update of the variance-covariance matrix and the mean vector*/
            state.sum_corr_param_matrix  = 
                update_sum_corr(std::move(state.sum_corr_param_matrix), parameters);
            state.sum_mean_param[0]+=parameters.epsilon[0];
            state.sum_mean_param[1]+=parameters.epsilon[2];
            state.sum_mean_param[2]+=parameters.epsilon[4];
            state.sum_mean_param[3]+=parameters.psi;
            state.sum_mean_param[4]+=parameters.transmissibility;
            state.sum_mean_param[5]+=parameters.susceptibility[0];
            state.sum_mean_param[6]+=parameters.susceptibility[3];
            state.sum_mean_param[7]+=parameters.susceptibility[6];
            state.sum_mean_param[8]+=parameters.init_pop;
            /*adjust variance for MCMC parameters*/
            if(k%1000==0)
            {
                /*update of the adaptive algorithm*/
                for(size_t i=0;i<state.emp_cov_matrix.size1();i++)
                {
                    state.emp_cov_matrix(i,i)=(state.sum_corr_param_matrix(i,i)-(state.sum_mean_param[i]*state.sum_mean_param[i])/k)/(k-1);
                    for(size_t j=0;j<i;j++)
                    {
                        state.emp_cov_matrix(i,j)=(state.sum_corr_param_matrix(i,j)-(state.sum_mean_param[i]*state.sum_mean_param[j])/k)/(k-1);
                        state.emp_cov_matrix(j,i)=state.emp_cov_matrix(i,j);
                    }
                }
                state.chol_emp_cov = cholesky_factorization(
                        state.emp_cov_matrix);

                state.past_acceptance=state.acceptance;
                state.acceptance=0;

                state.conv_scaling/=1.005;
            }
            return state;
        }

        parameter_set haario_adapt_scale(const parameter_set &current, 
                const bu::matrix<double> &chol_de, const bu::matrix<double> &chol_ini, int n, double beta, double adapt_scale, gsl_rng * gen)
        {
            parameter_set proposed;
            double normal_draw[9];
            double normal_add_draw[9], correlated_draw[9], correlated_fix[9];
            double unif1, unif2;
            int i, j, valid_flag;
            double un_moins_beta;

            un_moins_beta=1-beta;

            valid_flag=0;
            do
            {
                // TODO create random multivariate draw and use with
                // both chol_de and chol_ini
                /*drawing of the 9 N(0,1) samples using Box-Muller*/
                for(i=0;i<4;i++)
                {
                    unif1=R::runif(0,1);
                    unif2=R::runif(0,1);
                    normal_draw[i*2]=adapt_scale*2.38/3*sqrt(-2*log(unif1))*sin(twopi*unif2); /*3 = sqrt(9)*/
                    normal_draw[i*2+1]=adapt_scale*2.38/3*sqrt(-2*log(unif1))*cos(twopi*unif2);
                }

                unif1=R::runif(0,1);
                unif2=R::runif(0,1);
                normal_draw[8]=adapt_scale*2.38/3*sqrt(-2*log(unif1))*sin(twopi*unif2);
                normal_add_draw[8]=sqrt(-2*log(unif1))*cos(twopi*unif2);

                /*drawing of the 9 N(0,1) samples using Box-Muller*/
                for(i=0;i<4;i++)
                {
                    unif1=R::runif(0,1);
                    unif2=R::runif(0,1);
                    normal_add_draw[i*2]=sqrt(-2*log(unif1))*sin(twopi*unif2);
                    normal_add_draw[i*2+1]=sqrt(-2*log(unif1))*cos(twopi*unif2);
                }

                /*transforming the numbers generated with the Cholesky mat to get the correlated samples*/
                for(i=0;i<9;i++)
                    correlated_draw[i]=0;

                for(i=0;i<9;i++)
                    for(j=0;j<=i;j++)
                        correlated_draw[i]+=chol_de(i,j)*normal_draw[j];

                for(i=0;i<9;i++)
                    correlated_fix[i]=0;

                for(i=0;i<9;i++)
                    for(j=0;j<=i;j++)
                        correlated_fix[i]+=chol_ini(i,j)*normal_add_draw[j];

                /*new proposed values*/
                proposed.epsilon[0]=current.epsilon[0]+un_moins_beta*correlated_draw[0]+beta*correlated_fix[0];
                proposed.epsilon[1]=proposed.epsilon[0];
                proposed.epsilon[2]=current.epsilon[2]+un_moins_beta*correlated_draw[1]+beta*correlated_fix[1];
                proposed.epsilon[3]=proposed.epsilon[2];
                proposed.epsilon[4]=current.epsilon[4]+un_moins_beta*correlated_draw[2]+beta*correlated_fix[2];

                proposed.psi=current.psi+un_moins_beta*correlated_draw[3]+beta*correlated_fix[3];

                proposed.transmissibility=current.transmissibility+un_moins_beta*correlated_draw[4]+beta*correlated_fix[4];

                proposed.susceptibility[0]=current.susceptibility[0]+un_moins_beta*correlated_draw[5]+beta*correlated_fix[5];
                proposed.susceptibility[1]=proposed.susceptibility[0];
                proposed.susceptibility[2]=proposed.susceptibility[0];
                proposed.susceptibility[3]=current.susceptibility[3]+un_moins_beta*correlated_draw[6]+beta*correlated_fix[6];
                proposed.susceptibility[4]=proposed.susceptibility[3];
                proposed.susceptibility[5]=proposed.susceptibility[3];
                proposed.susceptibility[6]=current.susceptibility[6]+un_moins_beta*correlated_draw[7]+beta*correlated_fix[7];

                proposed.init_pop=current.init_pop+un_moins_beta*correlated_draw[8]+beta*normal_add_draw[8];

                // TODO: This could be moved to priors
                /*checking that the generating values are ok i.e. between 0 and 1 if probabilities*/
                if(proposed.epsilon[0] > 0)
                    if(proposed.epsilon[0] < 1)
                        if(proposed.epsilon[2] > 0)
                            if(proposed.epsilon[2] < 1)
                                if(proposed.epsilon[4] > 0)
                                    if(proposed.epsilon[4] < 1)
                                        if(proposed.psi >= 0)
                                            if(proposed.psi <= 1)
                                                if(proposed.transmissibility >= 0)
                                                    if(proposed.transmissibility <= 1)
                                                        if(proposed.susceptibility[0] >= 0)
                                                            if(proposed.susceptibility[0] <= 1)
                                                                if(proposed.susceptibility[3] >= 0)
                                                                    if(proposed.susceptibility[3] <= 1)
                                                                        if(proposed.susceptibility[6] >= 0)
                                                                            if(proposed.susceptibility[6] <= 1)
                                                                                if(proposed.init_pop<5)
                                                                                    valid_flag=1;
            }
            while(valid_flag==0);

            return proposed;
        }
    };
};


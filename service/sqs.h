/* sqs.h                                                           -*- C++ -*-
   Jeremy Barnes, 12 May 2013
   Copyright (c) 2013 Datacratic Inc.  All rights reserved.

   Client for Amazon's Simple Notification Service.
*/

#pragma once

#include "aws.h"
#include "http_rest_proxy.h"


namespace Datacratic {


/*****************************************************************************/
/* SQS API                                                                   */
/*****************************************************************************/
 
struct SqsApi : public AwsBasicApi {

    SqsApi(const std::string & protocol = "http",
           const std::string & region = "us-east-1");

    /** Parameters to create a queue */
    struct QueueParams {
        QueueParams()
            : delaySeconds(-1),
              maximumMessageSize(-1),
              messageRetentionPeriod(-1),
              receiveMessageWaitTimeSeconds(-1),
              visibilityTimeout(-1)
        {
        }

        int delaySeconds;
        int maximumMessageSize;
        int messageRetentionPeriod;
        std::string policy;
        int receiveMessageWaitTimeSeconds;
        int visibilityTimeout;
    };

    /** List the queue urls.
     */
    std::vector<std::string> listQueues(const std::string
                                        & queueNamePrefix = "");

    /** Create a queue.
     */
    std::string createQueue(const std::string & queueName,
                            const QueueParams & params = QueueParams());

    /** Delete a queue.
     */
    void deleteQueue(const std::string & queueUri);

    /** Return the URL for the given queue. */
    std::string getQueueUrl(const std::string & queueName,
                            const std::string & ownerAccountId = "");

    /** Set attributes of a queue. */
    void setQueueAttributes(const std::string & queueUri,
                            const QueueParams & attributes);

    struct QueueAttributes : public QueueParams {
        QueueAttributes()
            : QueueParams(),
              approximateNumberOfMessages(-1),
              approximateNumberOfMessagesDelayed(-1),
              approximateNumberOfMessagesNotVisible(-1)
        {}

        int approximateNumberOfMessages;
        int approximateNumberOfMessagesDelayed;
        int approximateNumberOfMessagesNotVisible;
        Date createdTimestamp;
        Date lastModifiedTimestamp;
        std::string queueArn;
    };

    /** Get the attributes of a queue. */
    QueueAttributes getQueueAttributes(const std::string & queueUri);

    /** Publish a message to a given SQS queue.  Returns the Message ID assigned
        by Amazon.

        By default, this will retry a failure 3 times before throwing an
        exception.

        \param queueName     The queue name to send the message to
        \param accountOwner  The account ID of the owner of the queue
        \param message       The message to be sent
        \param timeout       The timeout after which to retry
        \param delay         How many seconds (0-900) to delay the message within the
                             queue (see the AWS documentation).

        See also http://docs.aws.amazon.com/AWSSimpleQueueService/latest/APIReference/Query_QuerySendMessage.html
    */
    std::string
    sendMessage(const std::string & queueUrl,
                const std::string & message,
                int timeoutSeconds = 10,
                int delaySeconds = -1);

    /** Send multiple messages at once (10 at most). */
    void sendMessageBatch(const std::string & queueUri,
                          const std::vector<std::string> & messages,
                          int delaySeconds = -1);

    struct Message {
        bool isNull()
        { return messageId.empty(); }

        std::string body;
        std::string bodyMd5;
        std::string messageId;
        std::string receiptHandle;
        std::string senderId;
        Date sentTimestamp;
        int approximateReceiveCount;
        Date approximateFirstReceiveTimestamp;
    };

    Message receiveMessage(const std::string & queueUri,
                           int visibilityTimeout = -1,
                           int waitTimeSeconds = -1);

    std::vector<Message> receiveMessageBatch(const std::string & queueUri,
                                             int maxNumberOfMessages = 1,
                                             int visibilityTimeout = -1,
                                             int waitTimeSeconds = -1);

    /* Delete a message from a queue.

        \param queueUrl        The queue url to delete the message from
        \param receiptHandle   The receipt handle identifying the message.
                               Note: this is not the message id

       Failures are not reported by this operation.
    */
    void deleteMessage(const std::string & queueUri,
                       const std::string & receiptHandle);

    /* Delete messages from a queue.

        \param queueUrl         The queue url to delete the messages from
        \param receiptHandles   The receipt handles identifying the messages.
                                Note: those are not the message ids

       Failures are not reported by this operation.
    */
    void deleteMessageBatch(const std::string & queueUri,
                            const std::vector<std::string> & receiptHandles);


    /* Change the visibility of a message on the queue.

        \param queueUrl            The queue url the message belongs to
        \param receiptHandle       The receipt handle identifying the messages
                                   Note: this is not the message id
        \param visibilityTimeout   New timeout
    */
    void changeMessageVisibility(const std::string & queueUri,
                                 const std::string & receiptHandle,
                                 int visibilityTimeout);

    struct VisibilityPair {
        VisibilityPair(const std::string & handle, int timeout)
            : receiptHandle(handle), visibilityTimeout(timeout)
        {}

        std::string receiptHandle;
        int visibilityTimeout;
    };

    /* Change the visibility of a set of messages on the queue.

        \param queueUrl       The queue url the message belongs to
        \param visibilities   An array of VisibilityPair instances
    */
    void changeMessageVisibilityBatch(const std::string & queueUri,
                                      const std::vector<VisibilityPair>
                                      & visibilities);

    enum Rights {
        None = 0,
        SendMessage = 1 << 0,
        DeleteMessage = 1 << 1,
        ChangeMessageVisibility = 1 << 2,
        GetQueueAttributes = 1 << 3,
        GetQueueUrl = 1 << 4,
        All = (1 << 5) - 1,
    };
    static std::string rightToString(enum Rights rights);

    struct RightsPair {
        RightsPair(const std::string & principal, enum Rights rights)
            : principal(principal), rights(rights)
        {}

        std::string principal;
        enum Rights rights;
    };

    /* Add a permission to a queue

        \param queueUri   The queue url to modify permissions on
        \param label      The label identifying the permission set to add
        \param rights     A set of one or more permissions to assign to one
                          or more user principals
    */
    void addPermission(const std::string & queueUri,
                       const std::string & label,
                       const std::vector<RightsPair> & rights);

    /* Remove a permission to a queue

        \param queueUri   The queue url to modify permissions on
        \param label      The label identifying the permission set to remove
    */
    void removePermission(const std::string & queueUri,
                          const std::string & label);

    /** Turns a queue URI into a relative resource path for the HttpRestProxy */
    std::string getQueueResource(const std::string & queueUri) const;

};

} // namespace Datacratic
